#include <Arduino.h>
#include <SoftwareSerial.h>
#include "battery.h"
#include "config.h"
#include "site.hpp"

#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  WebServer server(80);
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  ESP8266WebServer server(80);
#endif


// SoftwareSerial serial(1, 2);
BatteryMonitor batteryMonitor(Serial, false);
BatteryState batteryState;


void setup() {
    #ifdef DEBUG
    Serial.begin(115200);
    Serial.println();
    Serial.print(F("Connecting to "));
    Serial.println(SSID);
    #endif

    #ifdef CREATE_APN
    WiFi.softAP(SSID, PASSWORD);
    #ifdef DEBUG
    Serial.print(F("created wifi network: "));
    Serial.print(SSID);
    Serial.print(F(" "));
    Serial.println(PASSWORD);
    #endif
    #else
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        #ifdef DEBUG
        Serial.print(F("."));
        #endif
    }
    #endif

    #ifdef DEBUG
    Serial.println(F("WiFi connected."));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
    #endif

    server.on(F("/"), handleRoot);
    
    server.onNotFound(handleNotFound);

    server.begin();
    #ifdef DEBUG
    Serial.println(F("HTTP server started"));
    #endif

    if (!MDNS.begin(F("battery"))) {
        #ifdef DEBUG
        Serial.println(F("Error setting up MDNS responder!"));
        #endif

    } else {
        #ifdef DEBUG
        Serial.println(F("mDNS responder started: http://battery.local"));
        #endif
    }
    
    initWithFakeData(batteryState);
}

void loop() {
    server.handleClient();
    MDNS.update();
}