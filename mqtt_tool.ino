#include "cy_mqtt.h"

const char* mqtt_subtopic_rl = "ATSH28/UG/G1/RL/1/set";
const char* mqtt_pubtopic_rl = "ATSH28/UG/G1/RL/1/state";
const char* mqtt_pubtopic = "ATSH28/UG/G1/SW/1/set";
const char* mqtt_subtopic_bell = "ATSH28/UG/G1/BELL/1/set";
const char* mqtt_pubtopic_bell = "ATSH28/UG/G1/BELL/1/state";


char gv_ptopic[MQTT_TOPSZ];
char gv_pbuffer[5];// buffer for reading the string to (needs to be large enough to take the longest string

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

void init_mqtt_local( ) {

  init_mqtt(gv_clientname);

  add_subtopic(mqtt_subtopic_rl, callback_mqtt);
  add_subtopic(mqtt_subtopic_bell, callback_mqtt);
}

void pub_bell(char *iv_state){
    client.publish(mqtt_pubtopic_bell, iv_state, true);
  //client.publish(mqtt_pubtopic_bell, "0", true);
}

void pub_mqtt_toggle() {
  client.publish(mqtt_pubtopic, "2");
}
