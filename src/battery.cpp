#include <SoftwareSerial.h>
#include "battery.h"
#include "config.h"

struct BatteryState;


BatteryMonitor::BatteryMonitor(SERIAL_TYPE &_serial, bool debugEnabled){
    batterySerial = &_serial;
    batterySerial->begin(115200);
    batterySerial->setTimeout(2000);
    debug = debugEnabled;
}

bool BatteryMonitor::sendCommand(const byte *cmd, int cmdLen) {
    if (debug) printBytes("BatteryMonitor request", cmd, cmdLen);

    int maxAtt = 1;
    for(int att = 1; att <=maxAtt; att++) {
        batterySerial->write(cmd, cmdLen);
        if(receiving()) {

            return true;
        }
        delay(500);
        if(debug) Serial.printf("failed (%d/%d)...\n", att, maxAtt);
    }

    return false;
}

bool BatteryMonitor::receiving() {
    unsigned long started = millis();
    while(!batterySerial->available()) {
        if(millis() - started >= 500) {
            if(debug) Serial.println("receive timeout");

            return false;
        }
    }
    memset(response, 0, responseSize);
    responsePos = 0;
    state = STATE_NONE;

    while (batterySerial->available()) {
        int currentByte = batterySerial->read();
        response[responsePos] = currentByte;

        bool save = true;
        switch (state) {
            case STATE_NONE:
                if (currentByte == 0x5A) {
                    state = STATE_HEADER2;
                    break;
                }

                if(debug) Serial.println("invalid state at header 1");
                if(debug) printBytes("bytes", response, 10);

                //hack to fix not receiving last byte
                return receiving();
            case STATE_HEADER2:
                if (currentByte != 0xA5) {
                    if(debug) Serial.println("invalid state at header 2");
                    if (debug) printBytes("bytes", response, 10);
                    save = false;
                    break;
                }

                state = STATE_LEN;
                break;
            case STATE_LEN:
                expectedLen = currentByte;
                state = STATE_SRCADDR;
                break;
            case STATE_SRCADDR:
                state = STATE_DSTADDR;
                break;
            case STATE_DSTADDR:
                state = STATE_CMD;
                break;
            case STATE_CMD:
                state = STATE_ARG;
                break;
            case STATE_ARG:
                //arg = currentByte
                state = STATE_PAYLOAD;
                payloadPos = 0;
                break;
            case STATE_PAYLOAD:
                state = STATE_PAYLOAD;
                payloadPos++;
                if (payloadPos >= expectedLen) {
                    state = STATE_CRC;
                    break;
                }
                break;
            case STATE_CRC:
                if (responsePos + 1 >= expectedLen + 9) {
                    if (debug) printBytes("stop by len " + String(expectedLen + 9), response, 40);

                    state = STATE_READY;
                    break;
                }
                break;
            default:
                Serial.printf("unknown state %d\n", state);
                panic();
        }
        if (!save) {
            memset(response, 0, responseSize);
            responsePos = 0;
            state = STATE_NONE;
            continue;
        }
        if (state == STATE_READY) {

            return verifyCrc();
        }

        responsePos++;
        if (responsePos >= responseSize) {
            if (debug) printBytes("invalid response size", response, 11);
            state = STATE_NONE;
            memset(response, 0, responseSize);
            responsePos = 0;

            return false;
        }
    }

    //hack to fix not receiving last byte
    if (state == STATE_CRC) {
        response[expectedLen + 9 - 1] = 0xFF;

        return verifyCrc();
    }

    if (debug) printBytes("not processing", response, 11);

    return false;
}

bool BatteryMonitor::readBatteryState(BatteryState &state) {

    if(!sendCommand(get_status, sizeof(get_status))) {
        state.error = F("error reading status");
        return false;
    } else{
        state.status = convertBytesToInt(response[8], response[7]);
    }

    if(!sendCommand(get_serial, sizeof(get_serial))) {
        state.error = F("error reading serial");
        return false;
    } else {
        byte serial[15] = {};
        memcpy(&serial, &response[7], 14 * sizeof(response[0])); serial[14] = 0x00;
        state.serial = String((char*)serial);
    }

    if(!sendCommand(get_remaining_capacity_perc, sizeof(get_remaining_capacity_perc))) {
        state.error = F("error reading remaining_capacity_perc");
        return false;
    } else {
        state.remaining_capacity_perc = convertBytesToInt(response[8], response[7]);
    }

    if(!sendCommand(get_remaining_capacity, sizeof(get_remaining_capacity))) {
        state.error = F("error reading remaining_capacity");
        return false;
    } else {
        state.remaining_capacity = convertBytesToInt(response[8], response[7]);
    }

    if(!sendCommand(get_factory_capacity, sizeof(get_factory_capacity))) {
        state.error = F("error reading factory_capacity");
        return false;
    } else {
        state.factory_capacity = convertBytesToInt(response[8], response[7]);
    }

    if(!sendCommand(get_actual_capacity, sizeof(get_actual_capacity))) {
        state.error = F("error reading actual_capacity");
        return false;
    } else {
        state.actual_capacity = convertBytesToInt(response[8], response[7]);
    }

    if(!sendCommand(get_current, sizeof(get_current))) {
        state.error = F("error reading current");
        return false;
    } else {
        state.current = double(convertBytesToInt(response[8], response[7])) * 10 / 1000;
    }

    if(!sendCommand(get_voltage, sizeof(get_voltage))) {
        state.error = F("error reading voltage");
        return false;
    } else {
        state.voltage = double(convertBytesToInt(response[8], response[7])) * 10 / 1000;
    }
    state.power = double (state.current * state.voltage);

    if(!sendCommand(get_temperature, sizeof(get_temperature))) {
        state.error = F("error reading temperature");
        return false;
    } else {
        state.temp_zone0 = response[7] - 20;
        state.temp_zone1 = response[8] - 20;
    }

    if(!sendCommand(get_cells_voltage, sizeof(get_cells_voltage))) {
        state.error = F("error reading cells_voltage");
        return false;
    } else {
        state.cell_voltage_cell0 = convertBytesToInt(response[8], response[7]);
        state.cell_voltage_cell1 = convertBytesToInt(response[10], response[9]);
        state.cell_voltage_cell2 = convertBytesToInt(response[12], response[11]);
        state.cell_voltage_cell3 = convertBytesToInt(response[14], response[13]);
        state.cell_voltage_cell4 = convertBytesToInt(response[16], response[15]);
        state.cell_voltage_cell5 = convertBytesToInt(response[18], response[17]);
        state.cell_voltage_cell6 = convertBytesToInt(response[20], response[19]);
        state.cell_voltage_cell7 = convertBytesToInt(response[22], response[21]);
        state.cell_voltage_cell8 = convertBytesToInt(response[24], response[23]);
        state.cell_voltage_cell9 = convertBytesToInt(response[26], response[25]);
    }
    state.uptime = long(millis() / 1000);

    return true;
}

int16_t BatteryMonitor::convertBytesToInt(byte byte1, byte byte2) {
    int16_t result = (byte1 << 8) | byte2;

    return result;
}

void BatteryMonitor::printBytes(const String &tag, const byte bytes[], int len) {
    Serial.print(tag + ": ");
    for (int i = 0; i < len; i++) {
        Serial.print("0x");
        if (bytes[i] <= 0xE) {
            Serial.print("0");
        }
        Serial.print(bytes[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");
}

bool BatteryMonitor::verifyCrc() {
    unsigned int cs = 0;
    for (int i = 2; i <= expectedLen + 6; i++) {
        cs = cs + response[i];
    }
    if (((response[expectedLen + 9 - 1] << 8) | response[expectedLen + 9 - 2]) == 0xFFFF - cs) {
        if (debug) printBytes("would parse", response, expectedLen + 9);

        return true;
    }
    if (debug) printBytes("invalid crc", response, expectedLen + 9);

    return false;
}
