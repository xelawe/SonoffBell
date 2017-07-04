/*
  Idea: https://tzapu.com/sonoff-firmware-boilerplate-tutorial/

    Flash Mode: DIO
    Flash Frequency: 40MHz
    Upload Using: Serial
    CPU Frequency: 80MHz
    Flash Size: 1M (64K SPIFFS)
    Debug Port: Disabled
    Debug Level: None
    Reset Method: ck
    Upload Speed: 115200
    Port: Your COM port connected to sonoff

   1MB flash size

   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14

   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

test git
*/

#define serdebug
#ifdef serdebug
#define DebugPrint(...) {  Serial.print(__VA_ARGS__); }
#define DebugPrintln(...) {  Serial.println(__VA_ARGS__); }
#else
#define DebugPrint(...) { }
#define DebugPrintln(...) { }
#endif

#define SONOFF_BUTTON    0
#define SONOFF_RELAY    12
#define PIN_RELAY       12
#define SONOFF_LED      13
#define PIN_LED         13
#define SONOFF_INPUT    14
#define PIN_INPUT       14

#define relStateOFF     LOW
#define relStateON      HIGH
#define LEDStateOFF     HIGH
#define LEDStateON      LOW
#define inpStatePressed LOW // Button pressed
#define inpStateReleased HIGH // Button relesaed

#include "wifi_tool.h"
#include "ota_tool.h"
#include "mqtt_tool.h"
#include <ArduinoOTA.h>

//for LED status
#include <Ticker.h>
Ticker ticker;
Ticker TickerBell;


const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;
int cmd = CMD_WAIT;

const int CMD_INPUT_CHANGE = 1;
int cmd_inp = CMD_WAIT;

int relayState = relStateOFF;

//inverted button state
int buttonState = HIGH;
int InputState = inpStateReleased;

static long startPress = 0;
boolean BellStarted = false;
boolean BellMute = false;
boolean TurnBellOff = false;

void tick()
{
  //toggle state
  int state = digitalRead(SONOFF_LED);  // get the current state of GPIO1 pin
  digitalWrite(SONOFF_LED, !state);     // set pin to the opposite state
}

void setState(int s) {

  relayState = s;

  DebugPrintln(relayState);

  digitalWrite(PIN_RELAY, relayState);
  if (relayState == relStateOFF) {
    digitalWrite(PIN_LED, LEDStateOFF);
    client.publish(mqtt_pubtopic_rl, "0", true);
  }
  else {
    digitalWrite(PIN_LED, LEDStateON);
    client.publish(mqtt_pubtopic_rl, "1", true);
  }

}

void turnOn() {
  setState(relStateON);
}

void turnOff() {
  setState(relStateOFF);
}

void toggleState() {
  cmd = CMD_BUTTON_CHANGE;
}

void toggleInput() {
  cmd_inp = CMD_INPUT_CHANGE;
}

void toggle() {
  Serial.println("toggle state");
  relayState = relayState == relStateOFF ? relStateON : relStateOFF;
  setState(relayState);
}

void restart() {
  ESP.reset();
  delay(1000);
}

void reset() {
  //reset wifi credentials
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

void callback_mqtt(char* topic, byte* payload, unsigned int length) {
  DebugPrint("Message arrived [");
  DebugPrint(topic);
  DebugPrint("] ");
  for (int i = 0; i < length; i++) {
    DebugPrint((char)payload[i]);
  }
  DebugPrintln();

  // Switch on the LED if an 1 was received as first character
  switch ((char)payload[0]) {
    case '0':
      turnOff();
      break;
    case '1':
      turnOn();
      break;
    case '2':
      toggle();
      break;
  }
}

void BellStart() {
  // Bell already started?
  if (BellStarted == true) {
    return;
  }

  // should Bell be muted?
  if (BellMute == true) {
    return;
  }

  BellStarted = true;
  // turn Bell Relay on
  turnOn();

  // start Ticker to turn off Bell Relay
  TickerBell.attach(1, BellStop);

  client.publish(mqtt_pubtopic_bell, "1", true);

}

void BellStop() {
  TickerBell.detach();
  TurnBellOff = true;
}


void BellTurnOff() {
  // turn off Bell Relays
  turnOff();

  // Wenn stopped, Bell should have a Pause, if not already muted
  if (BellMute == false) {

    BellMute = true;

    // start Ticker to unmute Bell
    TickerBell.attach(1, BellUnmute);
  }

  BellStarted = false;
  client.publish(mqtt_pubtopic_bell, "0", true);
}



void BellUnmute() {
  TickerBell.detach();
  BellMute = false;
}

void setup()
{
  Serial.begin(115200);

  DebugPrintln("\n" + String(__DATE__) + ", " + String(__TIME__) + " " + String(__FILE__));

  //set led pin as output
  pinMode(SONOFF_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  char *hostname = "SonoffBell";

  wifi_init(hostname);

  init_ota(hostname);

  //if you get here you have connected to the WiFi
  ticker.detach();

  //setup button
  pinMode(SONOFF_BUTTON, INPUT);
  attachInterrupt(SONOFF_BUTTON, toggleState, CHANGE);

  //setup relay
  pinMode(SONOFF_RELAY, OUTPUT);

  //setup input
  pinMode(SONOFF_INPUT, INPUT_PULLUP);
  InputState = digitalRead(SONOFF_INPUT);
  attachInterrupt(SONOFF_INPUT, toggleInput, CHANGE);

  init_mqtt(callback_mqtt);

  turnOff();

  Serial.println("done setup");

}


void loop()
{

  check_ota();

  check_mqtt();

  if (TurnBellOff == true) {
    TurnBellOff = false;
    BellTurnOff();
  }

  // Button pressed -> start Bell
  switch (cmd_inp) {
    case CMD_WAIT:
      break;
    case CMD_INPUT_CHANGE:
      int currentStateInp = digitalRead(PIN_INPUT);
      if (currentStateInp != InputState) {
        if (currentStateInp == inpStatePressed) {
          BellStart();
        }
        InputState = currentStateInp;

        if (InputState == inpStatePressed) {
          //client.publish(mqtt_pubtopic_wl, "0", true);
        }
        else {
          //client.publish(mqtt_pubtopic_wl, "1", true);
        }
        break;
      }
  }

  switch (cmd) {
    case CMD_WAIT:
      break;
    case CMD_BUTTON_CHANGE:
      int currentState = digitalRead(SONOFF_BUTTON);
      if (currentState != buttonState) {
        if (buttonState == LOW && currentState == HIGH) {
          long duration = millis() - startPress;
          if (duration < 50) {
            Serial.println("too short press - no action");
          } else if (duration < 2000) {
            Serial.println("short press - toggle relay");
            toggle();
          } else if (duration < 10000) {
            Serial.println("medium press - reset");
            restart();
          } else if (duration < 60000) {
            Serial.println("long press - reset settings");
            reset();
          }
        } else if (buttonState == HIGH && currentState == LOW) {
          startPress = millis();
        }
        buttonState = currentState;
      }
      break;
  }

  //  check_time();
  delay(100);

}
