#pragma once
#include <ESP8266WiFi.h>
#include "config.h"

struct BatteryState {
    int16_t status; // 0=Discharge, 1=Charge, 2=Idle
    String serial;
    int16_t remaining_capacity_perc;
    int16_t remaining_capacity;
    int16_t factory_capacity;
    int16_t actual_capacity;
    double current;
    double voltage;
    double power;
    int8_t temp_zone0, temp_zone1;
    int16_t cell_voltage_cell0, cell_voltage_cell1, cell_voltage_cell2, cell_voltage_cell3,
            cell_voltage_cell4, cell_voltage_cell5, cell_voltage_cell6, cell_voltage_cell7,
            cell_voltage_cell8, cell_voltage_cell9;
    long uptime;
    String error;
};


class BatteryMonitor {
public:
    bool debug = false;
    SERIAL_TYPE *batterySerial;

    BatteryMonitor(SERIAL_TYPE &_serial, bool debugEnabled);

    // DynamicJsonDocument readBattery();
    bool readBatteryState(BatteryState &state);

    bool sendCommand(const byte cmd[], int cmdLen);

    bool receiving();

private:

    const byte get_status[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x30, 0x02, 0x89, 0xff,};
    const byte get_serial[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x10, 0x0e, 0x9d, 0xff,};
    const byte get_remaining_capacity_perc[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x32, 0x02, 0x87, 0xff,};
    const byte get_remaining_capacity[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x31, 0x02, 0x88, 0xff,};
    const byte get_actual_capacity[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x19, 0x02, 0xa0, 0xff,};
    const byte get_factory_capacity[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x18, 0x02, 0xa1, 0xff,};
    const byte get_current[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x33, 0x02, 0x86, 0xff,};
    const byte get_voltage[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x34, 0x02, 0x85, 0xff,};
    const byte get_cells_voltage[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x40, 0x14, 0x67, 0xff,};
    const byte get_temperature[10] = {0x5a, 0xa5, 0x01, 0x20, 0x22, 0x01, 0x35, 0x02, 0x84, 0xff,};

    enum states {
        STATE_NONE = 0,
        STATE_HEADER1 = 1,
        STATE_HEADER2 = 2,
        STATE_LEN = 3,
        STATE_SRCADDR = 4,
        STATE_DSTADDR = 5,
        STATE_CMD = 6,
        STATE_ARG = 7,
        STATE_PAYLOAD = 8,
        STATE_CRC = 9,
        STATE_READY = 10,
    };
    static const int responseSize = 64;
    byte response[responseSize]{};
    int expectedLen{};
    int payloadPos{};
    int responsePos{};
    states state = STATE_NONE;

    int16_t convertBytesToInt(byte byte1, byte byte2);

    void printBytes(const String &tag, const byte bytes[], int len);

    bool verifyCrc();
};

