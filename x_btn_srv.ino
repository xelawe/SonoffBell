const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;
volatile int cmd = CMD_WAIT;

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

void ICACHE_RAM_ATTR toggleState() {
  cmd = CMD_BUTTON_CHANGE;
}

void init_btn() {
  //setup button
  pinMode(SONOFF_BUTTON, INPUT);
  attachInterrupt(SONOFF_BUTTON, toggleState, CHANGE);
}

void check_btn() {
  switch (cmd) {
    case CMD_WAIT:
      break;
    case CMD_BUTTON_CHANGE:
      {
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
        cmd = CMD_WAIT;
      }
      break;
  }
}

