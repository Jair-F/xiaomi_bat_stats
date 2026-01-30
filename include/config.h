#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#define SSID F("Home")
#define PASSWORD F("JaDaLiAdJa1209!?,")
#define LOCAL_DNS_NAME F("battery")

#define DEBUG
#ifdef DEBUG
#define SERIAL_TYPE SoftwareSerial
#else
#define SERIAL_TYPE HardwareSerial
#endif
