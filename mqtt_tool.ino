#include "cy_mqtt.h"

//const char* mqtt_subtopic_rl = "ATSH28/UG/G1/RL/1/set";
const char* mqtt_pubtopic_rl = "ATSH28/UG/G1/RL/1/state";

const char* mqtt_pubtopic = "ATSH28/UG/G1/SW/1/set";

//const char* mqtt_subtopic_bell = "ATSH28/UG/G1/BELL/1/set";
const char* mqtt_pubtopic_bell = "ATSH28/UG/G1/BELL/1/state";


char gv_stopic_bell[MQTT_TOPSZ];
char gv_stopic_rl[MQTT_TOPSZ];
const char gc_stopic_bell[] PROGMEM = "bell";
const char gc_stopic_rl[] PROGMEM = "rl";
const char* const gt_stopic[] PROGMEM = { gc_stopic_bell, gc_stopic_rl};
char gv_sbuffer[7];// buffer for reading the string to (needs to be large enough to take the longest string

char gv_ptopic[MQTT_TOPSZ];
char gv_pbuffer[5];// buffer for reading the string to (needs to be large enough to take the longest string


const char gc_cmd_off[] PROGMEM = "off";
const char gc_cmd_on[] PROGMEM = "on";
const char gc_cmd_stop[] PROGMEM = "stop";
const char* const gt_cmd[] PROGMEM = { gc_cmd_off, gc_cmd_on, gc_cmd_stop };

char *get_stopic_ix( int ix ) {
  strcpy_P(gv_sbuffer, (char*)pgm_read_dword(&(gt_stopic[ix])));
  //strcpy_P(gv_buffer, );
  return gv_sbuffer;
}

char *get_cmd_ix( int ix ) {
  strcpy_P(gv_pbuffer, (char*)pgm_read_dword(&(gt_cmd[ix])));
  //strcpy_P(gv_buffer, );
  return gv_pbuffer;
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

void init_mqtt_local( ) {

  init_mqtt(gv_clientname);

  //add_subtopic(mqtt_subtopic_rl, callback_mqtt);
  //add_subtopic(mqtt_subtopic_bell, callback_mqtt);
  add_subtopic(mqtt_GetTopic_P(gv_stopic_bell, 0, gv_clientname, get_stopic_ix(0)), callback_mqtt);
  add_subtopic(mqtt_GetTopic_P(gv_stopic_rl, 0, gv_clientname, get_stopic_ix(1)), callback_mqtt);
}

void pub_bell(int cmd) {
  //client.publish(mqtt_pubtopic_bell, iv_state, true);
  //client.publish(mqtt_pubtopic_bell, "0", true);

  if (!client.publish(mqtt_GetTopic_P(gv_ptopic, 1, gv_clientname, get_stopic_ix(0)), get_cmd_ix(cmd), true)) {
    DebugPrintln(F("pub failed!"));
  } else {
    DebugPrintln(F("pub ok!"));
  }
}

void pub_mqtt_toggle() {
  client.publish(mqtt_pubtopic, "2");
}
