#pragma once

#include <Controllino.h>

//max 25 bytes
const char    DEVICENAME[] = "sps_ffw1";
const String  VERSION = "1.1";
const uint8_t MAC[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
const uint8_t IP[] = {10, 0, 110, 240}; 

const uint8_t MQTT_IP[] = {10, 0, 130, 10};
const int     MQTT_PORT = 1883;
const char    MQTT_USER[] = "";
const char    MQTT_PASS[] = "";

// GATEs ==>

const int GATE_MOVING_TIME = 15000;
const int GATE_WARNING_TIME = 5000;
const int GATE_CMD_DELAY = 1000;

// CMD_UP CMD_DOWN CMD_STOP CMD_WARN STATE_UP STATE_DOWN
const uint8_t GATE_1_CONFIG[6] = {CONTROLLINO_R6,CONTROLLINO_R7,CONTROLLINO_D0,CONTROLLINO_R1,CONTROLLINO_A0,CONTROLLINO_A1};
const uint8_t GATE_2_CONFIG[6] = {CONTROLLINO_R8,CONTROLLINO_R9,CONTROLLINO_D1,CONTROLLINO_R2,CONTROLLINO_A2,CONTROLLINO_A3};
const uint8_t GATE_3_CONFIG[6] = {CONTROLLINO_R10,CONTROLLINO_R11,CONTROLLINO_D2,CONTROLLINO_R3,CONTROLLINO_A4,CONTROLLINO_A5};
const uint8_t GATE_4_CONFIG[6] = {CONTROLLINO_R12,CONTROLLINO_R13,CONTROLLINO_D3,CONTROLLINO_R4,CONTROLLINO_A6,CONTROLLINO_A7};
const uint8_t GATE_5_CONFIG[6] = {CONTROLLINO_R14,CONTROLLINO_R15,CONTROLLINO_D4,CONTROLLINO_R5,CONTROLLINO_A8,CONTROLLINO_A9};

// <== GATEs

// LIGHT ==>
// PIN_IN PIN_OUT MODE
const uint8_t LIGHT_OUTSIDE_CONFIG[2] = {CONTROLLINO_I18, CONTROLLINO_D23};
// <== LIGHT
