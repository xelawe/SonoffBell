#define relStateOFF     LOW
#define relStateON      HIGH
#define LEDStateOFF     HIGH
#define LEDStateON      LOW
#define inpStatePressed LOW // Button pressed
#define inpStateReleased HIGH // Button relesaed

Ticker TickerBell;

int volatile relayState = relStateOFF;

//inverted button state
int buttonState = HIGH;

static long startPress = 0;
boolean BellStarted = false;
boolean BellMute = false;
boolean TurnBellOff = false;


void ICACHE_RAM_ATTR setState(int s) {

  relayState = s;

  DebugPrintln(relayState);

  digitalWrite(PIN_RELAY, relayState);
  if (relayState == relStateOFF) {
    digitalWrite(PIN_LED, LEDStateOFF);
    //client.publish(mqtt_pubtopic_rl, "0", true);
  }
  else {
    digitalWrite(PIN_LED, LEDStateON);
    //client.publish(mqtt_pubtopic_rl, "1", true);
  }

}

void ICACHE_RAM_ATTR turnOn() {
  setState(relStateON);
}

void turnOff() {
  setState(relStateOFF);
}

void toggle() {
  Serial.println("toggle state");
  relayState = relayState == relStateOFF ? relStateON : relStateOFF;
  setState(relayState);
}

void BellStop() {
  TickerBell.detach();
  TurnBellOff = true;
}

void BellUnmute() {
  TickerBell.detach();
  BellMute = false;
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

  pub_bell("1");
  pub_bell("0");
}

void ICACHE_RAM_ATTR BellStart() {
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
  TickerBell.attach_ms(700, BellStop);

  //client.publish(mqtt_pubtopic_bell, "1", true);

}

void ICACHE_RAM_ATTR setInputPressed() {

  BellStart();
}

void init_input() {
  //setup input
  pinMode(SONOFF_INPUT, INPUT_PULLUP);

  attachInterrupt(SONOFF_INPUT, setInputPressed, FALLING);
}
