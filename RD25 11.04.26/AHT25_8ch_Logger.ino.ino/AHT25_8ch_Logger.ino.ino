/*
 * RD25 - быстрая прошивка с логированием (WiFi/веб-сервер отключены)
 * Версия: 9.1
 */

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <SPIFFS.h>
#include <Preferences.h>

#include "config.h"
#include "types.h"
#include "sensors.h"
#include "logging.h"

// ======================= ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ =======================
Preferences prefs;
unsigned long lastSensorRead = 0;
unsigned long lastLogWrite = 0;
unsigned long logStartTime = 0;
bool loggingEnabled = false;
bool autoStopEnabled = false;
unsigned long logInterval = 60000;   // 1 минута
unsigned long logDuration = 0;
String currentLogFile = "";

unsigned long pollingInterval = 1000;

// ======================= ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ =======================
void sendDataToPC() {
    SensorData* data = sensorManager.getData();
    Serial.print("DATA:");
    for (int i = 0; i < 8; i++) {
        if (data->present[i]) {
            Serial.print(data->temperature[i], 2);
            Serial.print(",");
            Serial.print(data->humidity[i], 2);
        } else {
            Serial.print("---,---");
        }
        if (i < 7) Serial.print(",");
    }
    Serial.println();
}

void processSerialCommand(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;

    if (cmd.startsWith("SET_INTERVAL:")) {
        unsigned long newInterval = cmd.substring(13).toInt();
        if (newInterval >= 1000 && newInterval <= 86400000) {
            pollingInterval = newInterval;
            Serial.print("OK: Polling interval = ");
            Serial.println(pollingInterval);
            lastSensorRead = millis() - pollingInterval + 100;
        } else {
            Serial.println("ERROR: Interval 1000-86400000 ms");
        }
    }
    else if (cmd.equals("READ_SENSORS_NOW")) {
        sensorManager.readAllSensors();
        sendDataToPC();
    }
    else if (cmd.equals("SCAN_SENSORS")) {
        sensorManager.scanSensors();
        sendDataToPC();
    }
    else if (cmd.equals("START_LOGGING")) {
        if (!loggingEnabled) {
            loggingEnabled = true;
            autoStopEnabled = (logDuration > 0);
            logStartTime = millis();
            currentLogFile = "";
            Serial.println("LOGGING_STARTED");
        } else {
            Serial.println("ERROR: Logging already active");
        }
    }
    else if (cmd.equals("STOP_LOGGING")) {
        if (loggingEnabled) {
            loggingEnabled = false;
            autoStopEnabled = false;
            logManager.finalizeLogFile();
            Serial.println("LOGGING_STOPPED");
        } else {
            Serial.println("ERROR: Logging not active");
        }
    }
    else if (cmd.startsWith("SET_LOG_INTERVAL:")) {
        int sec = cmd.substring(17).toInt();
        if (sec >= 1 && sec <= 86400) {
            logInterval = sec * 1000UL;
            Serial.println("OK: Log interval set to " + String(sec) + " sec");
        } else {
            Serial.println("ERROR: Log interval must be 1-86400 sec");
        }
    }
    else if (cmd.startsWith("SET_LOG_DURATION:")) {
        int sec = cmd.substring(17).toInt();
        if (sec >= 0 && sec <= 86400*365) {
            logDuration = sec * 1000UL;
            Serial.println("OK: Log duration set to " + String(sec) + " sec");
        } else {
            Serial.println("ERROR: Invalid duration");
        }
    }
    else if (cmd.equals("GET_LOG_REMAINING")) {
        if (loggingEnabled && autoStopEnabled && logDuration > 0) {
            unsigned long elapsed = millis() - logStartTime;
            long remaining = (elapsed >= logDuration) ? 0 : (logDuration - elapsed) / 1000;
            Serial.print("LOG_REMAINING:");
            Serial.println(remaining);
        } else {
            Serial.println("LOG_REMAINING:0");
        }
    }
    else if (cmd.equals("GET_SETTINGS")) {
        Serial.print("SETTINGS: polling=");
        Serial.print(pollingInterval);
        Serial.print(",log_interval=");
        Serial.print(logInterval);
        Serial.print(",log_duration=");
        Serial.print(logDuration);
        Serial.print(",logging=");
        Serial.println(loggingEnabled ? "1" : "0");
    }
    else if (cmd.equals("LIST_FILES")) {
        logManager.listFilesSerial();
    }
    else if (cmd.startsWith("DOWNLOAD_FILE:")) {
        String filename = cmd.substring(14);
        filename.trim();
        if (filename.length() == 0) {
            Serial.println("ERROR: empty filename");
            return;
        }
        if (!filename.startsWith("/")) filename = "/" + filename;
        File f = SPIFFS.open(filename, FILE_READ);
        if (!f) {
            Serial.println("ERROR: file not found");
            return;
        }
        Serial.print("FILE_START:");
        Serial.println(filename);
        while (f.available()) {
            Serial.write(f.read());
        }
        f.close();
        Serial.println();
        Serial.println("FILE_END");
    }
    else if (cmd.equals("CLEAR_FILES")) {
        logManager.clearAllLogs();
        Serial.println("CLEAR_COMPLETE");
    }
    else {
        Serial.println("UNKNOWN_CMD");
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== RD25 (с логированием, без WiFi) ===");
    Wire.begin();
    Wire.setClock(400000);
    Wire.setTimeout(100);
    Serial.println("I2C 400kHz");

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
    } else {
        Serial.println("SPIFFS ready");
        logManager.cleanupOldLogs();
    }

    sensorManager.scanSensors();
    Serial.println("Ready. Commands: START_LOGGING, STOP_LOGGING, SET_LOG_INTERVAL:sec, SET_LOG_DURATION:sec, LIST_FILES, DOWNLOAD_FILE:name, CLEAR_FILES");
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        processSerialCommand(cmd);
    }

    // Опрос датчиков и отправка данных
    if (millis() - lastSensorRead >= pollingInterval) {
        sensorManager.readAllSensors();
        sendDataToPC();
        lastSensorRead = millis();
    }

    // Логирование (если включено)
    if (loggingEnabled) {
        // Проверка автоостановки
        if (autoStopEnabled && logDuration > 0 && (millis() - logStartTime >= logDuration)) {
            loggingEnabled = false;
            autoStopEnabled = false;
            logManager.finalizeLogFile();
            Serial.println("LOGGING_STOPPED");
        }
        // Запись по интервалу
        if (millis() - lastLogWrite >= logInterval) {
            logManager.writeLogToFile();
            lastLogWrite = millis();
        }
    }
    delay(5);
}