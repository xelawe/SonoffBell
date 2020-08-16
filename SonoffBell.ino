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
   gpio 12 - relaye
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

*/

//#define USE_SERIAL Serial
#include <cy_serdebug.h>
#include <cy_serial.h>

#define SONOFF_BUTTON    0
#define SONOFF_RELAY    12
#define PIN_RELAY       12
#define SONOFF_LED      13
#define PIN_LED         13
#define SONOFF_INPUT    14
#define PIN_INPUT       14



#include "cy_wifi.h"
#include "cy_ota.h"

//for LED status
#include <Ticker.h>
Ticker ticker;

//const char* gv_hostname = "SonoffBell";
const char *gc_hostname = "SonoffBell";

boolean gv_check_net = false;


void tick_net_checks() {
  gv_check_net = true;
}

void tick()
{
  //toggle state
  int state = digitalRead(SONOFF_LED);  // get the current state of GPIO1 pin
  digitalWrite(SONOFF_LED, !state);     // set pin to the opposite state
}


void setup()
{
  cy_serial::start(__FILE__);

  init_bell( );

  //set led pin as output
  pinMode(SONOFF_LED, OUTPUT);
  
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //wifi_init(gv_hostname);
  wifi_init(gc_hostname);

  //init_ota(gv_hostname);
  init_ota(gv_clientname);

  //if you get here you have connected to the WiFi
  ticker.detach();

  init_btn();

  //init_mqtt(callback_mqtt);
  init_mqtt_local( );

  ticker.attach(2, tick_net_checks);

  Serial.println("done setup");

}
