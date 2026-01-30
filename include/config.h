#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#define CREATE_APN
#define SSID F("Battery")
#define PASSWORD F("")
#define LOCAL_DNS_NAME F("battery")

// #define DEBUG
#ifdef DEBUG
#define SERIAL_TYPE SoftwareSerial
#else
#define SERIAL_TYPE HardwareSerial
#endif
