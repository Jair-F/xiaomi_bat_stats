#pragma once
#include <Arduino.h>
#include "battery.h"

#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  extern WebServer server;
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  extern ESP8266WebServer server;
#endif
extern BatteryMonitor batteryMonitor;
extern BatteryState batteryState;

String formatTime(long seconds) {
    long h = seconds / 3600;
    long m = (seconds % 3600) / 60;
    long s = seconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02ld:%02ld:%02ld", h, m, s);
    return String(buf);
}

String getHealthColor(int perc) {
    if (perc > 50) return "#2ecc71"; // Green
    if (perc > 20) return "#f1c40f"; // Yellow
    return "#e74c3c"; // Red
}

String getBatteryHTML(BatteryState &state) {
    String statusText = (state.status == 1) ? "Charging" : (state.status == 0 ? "Discharging" : "Idle");
    String uptimeStr = formatTime(state.uptime);
    String barColor = getHealthColor(state.remaining_capacity_perc);
    int soh = (state.factory_capacity > 0) ? (state.actual_capacity * 100 / state.factory_capacity) : 0;

    String html = R"rawliteral(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>BMS Monitor</title>
  <style>
    :root { --bg:#121212; --card:#1e1e1e; --text:#ddd; --accent:#3498db; --border:#333; }
    body { font-family:sans-serif; background:var(--bg); color:var(--text); margin:0; padding:10px; }
    h2 { color:var(--accent); text-align:center; margin:10px 0; }
    .grid { display:grid; grid-template-columns:repeat(auto-fit, minmax(300px, 1fr)); gap:10px; }
    .card { background:var(--card); border:1px solid var(--border); border-radius:8px; padding:15px; }
    .head { border-bottom:1px solid var(--border); padding-bottom:5px; margin-bottom:10px; color:#888; text-transform:uppercase; font-size:0.85rem; letter-spacing:1px; }
    .row { display:flex; justify-content:space-between; margin-bottom:8px; font-size:0.9rem; }
    .val { font-family:monospace; font-weight:bold; font-size:1.1rem; }
    .bar-bg { background:#333; height:24px; border-radius:12px; overflow:hidden; margin:15px 0; position:relative; }
    .bar-fill { height:100%; text-align:center; line-height:24px; color:#000; font-weight:bold; font-size:0.8rem; transition:width 0.3s; }
    .cells { display:grid; grid-template-columns:repeat(5, 1fr); gap:5px; text-align:center; }
    .c-box { background:#252525; padding:4px; border-radius:4px; font-size:0.9rem; }
    .c-lbl { display:block; font-size:0.6rem; color:#666; }
    .err { background:#e74c3c33; color:#e74c3c; padding:10px; border-radius:5px; text-align:center; display:{{SHOW_ERR}}; margin-top:10px; border:1px solid #e74c3c; }
    .btn { display:block; width:100%; padding:12px; background:var(--accent); color:#fff; border:none; border-radius:5px; margin-top:15px; cursor:pointer; font-size:1rem; }
  </style>
</head>
<body>
  <h2>Battery Monitor</h2>
  <div class="card">
    <div class="head">Overview</div>
    <div class="bar-bg">
      <div class="bar-fill" style="width:{{PERC}}%; background:{{COLOR}};">{{PERC}}%</div>
    </div>
    <div class="row"><span>Status</span><span class="val">{{STATUS}}</span></div>
    <div class="row"><span>Capacity</span><span class="val">{{REM}} / {{ACT}} mAh</span></div>
    <div class="row"><span>Health (SOH)</span><span class="val">{{SOH}}%</span></div>
  </div>
  <br>
  <div class="grid">
    <div class="card">
      <div class="head">Power Metrics</div>
      <div class="row"><span>Voltage</span><span class="val">{{VOLT}} V</span></div>
      <div class="row"><span>Current</span><span class="val">{{CURR}} A</span></div>
      <div class="row"><span>Power</span><span class="val">{{POW}} W</span></div>
    </div>
    <div class="card">
      <div class="head">System & Temp</div>
      <div class="row"><span>Serial</span><span class="val">{{SER}}</span></div>
      <div class="row"><span>Uptime</span><span class="val">{{UP}}</span></div>
      <div class="row"><span>Temp Z0</span><span class="val">{{T0}} &deg;C</span></div>
      <div class="row"><span>Temp Z1</span><span class="val">{{T1}} &deg;C</span></div>
    </div>
  </div>
  <br>
  <div class="card">
    <div class="head">Cell Voltages (mV)</div>
    <div class="cells">
      <div class="c-box"><span class="c-lbl">1</span>{{C0}}</div>
      <div class="c-box"><span class="c-lbl">2</span>{{C1}}</div>
      <div class="c-box"><span class="c-lbl">3</span>{{C2}}</div>
      <div class="c-box"><span class="c-lbl">4</span>{{C3}}</div>
      <div class="c-box"><span class="c-lbl">5</span>{{C4}}</div>
      <div class="c-box"><span class="c-lbl">6</span>{{C5}}</div>
      <div class="c-box"><span class="c-lbl">7</span>{{C6}}</div>
      <div class="c-box"><span class="c-lbl">8</span>{{C7}}</div>
      <div class="c-box"><span class="c-lbl">9</span>{{C8}}</div>
      <div class="c-box"><span class="c-lbl">10</span>{{C9}}</div>
    </div>
  </div>
  <div class="err">⚠️ {{ERR_MSG}}</div>
  <button class="btn" onclick="location.reload()">Refresh Data</button>
</body>
</html>)rawliteral";

    html.replace("{{PERC}}", String(state.remaining_capacity_perc));
    html.replace("{{COLOR}}", barColor);
    html.replace("{{STATUS}}", statusText);
    html.replace("{{REM}}", String(state.remaining_capacity));
    html.replace("{{ACT}}", String(state.actual_capacity));
    html.replace("{{SOH}}", String(soh));
    html.replace("{{VOLT}}", String(state.voltage, 2));
    html.replace("{{CURR}}", String(state.current, 2));
    html.replace("{{POW}}", String(state.power, 2));
    html.replace("{{SER}}", state.serial);
    html.replace("{{UP}}", uptimeStr);
    html.replace("{{T0}}", String(state.temp_zone0));
    html.replace("{{T1}}", String(state.temp_zone1));
    html.replace("{{C0}}", String(state.cell_voltage_cell0));
    html.replace("{{C1}}", String(state.cell_voltage_cell1));
    html.replace("{{C2}}", String(state.cell_voltage_cell2));
    html.replace("{{C3}}", String(state.cell_voltage_cell3));
    html.replace("{{C4}}", String(state.cell_voltage_cell4));
    html.replace("{{C5}}", String(state.cell_voltage_cell5));
    html.replace("{{C6}}", String(state.cell_voltage_cell6));
    html.replace("{{C7}}", String(state.cell_voltage_cell7));
    html.replace("{{C8}}", String(state.cell_voltage_cell8));
    html.replace("{{C9}}", String(state.cell_voltage_cell9));

    if (state.error.length() > 0 && state.error != "None") {
        html.replace("{{SHOW_ERR}}", "block");
        html.replace("{{ERR_MSG}}", state.error);
    }
    else {
        html.replace("{{SHOW_ERR}}", "none");
        html.replace("{{ERR_MSG}}", "");
    }
    return html;
}

void initWithFakeData(BatteryState& batteryState) {

    batteryState.uptime = millis() / 1000;
    
    batteryState.remaining_capacity_perc = -1;
    batteryState.status = -1;
    batteryState.serial = "----------";
    batteryState.factory_capacity = -1;
    batteryState.actual_capacity = -1;
    batteryState.remaining_capacity = -1;
    batteryState.voltage = -1;
    batteryState.current = -1;
    batteryState.power = -1;
    batteryState.temp_zone0 = -1;
    batteryState.temp_zone1 = -1;
    batteryState.error = F("Fake Error - Testing");
    
    batteryState.cell_voltage_cell0 = -1;
    batteryState.cell_voltage_cell1 = -1;
    batteryState.cell_voltage_cell2 = -1;
    batteryState.cell_voltage_cell3 = -1;
    batteryState.cell_voltage_cell4 = -1;
    batteryState.cell_voltage_cell5 = -1;
    batteryState.cell_voltage_cell6 = -1;
    batteryState.cell_voltage_cell7 = -1;
    batteryState.cell_voltage_cell8 = -1;
    batteryState.cell_voltage_cell9 = -1;
}


void handleRoot() {
    batteryMonitor.readBatteryState(batteryState);

    String htmlPage = getBatteryHTML(batteryState);
    
    server.send(200, "text/html", htmlPage);
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}
