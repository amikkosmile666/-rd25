#ifndef WIFI_NTP_H
#define WIFI_NTP_H

#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "types.h"

class WiFiManager {
public:
    static bool connectWiFi() {
        if (strlen(ssid) == 0) {
            Serial.println("No SSID, skip WiFi connection");
            return false;
        }
        Serial.println("Подключение к WiFi...");
        WiFi.mode(WIFI_STA);
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.mode(WIFI_OFF);
        }
        return (WiFi.status() == WL_CONNECTED);
    }
    
    static void checkConnection() {
        if (strlen(ssid) == 0) return;
        if (WiFi.getMode() == WIFI_OFF) return;
        static unsigned long lastWiFiCheck = 0;
        if (millis() - lastWiFiCheck >= 10000) {
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("Автовосстановление WiFi...");
                connectWiFi();
            }
            lastWiFiCheck = millis();
        }
    }
    
    static String getStatus() {
        return (WiFi.status() == WL_CONNECTED) ? "ПОДКЛЮЧЕНО" : "ОТКЛЮЧЕНО";
    }
    
    static String getIP() {
        return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "N/A";
    }
    
    static String getRSSI() {
        return (WiFi.status() == WL_CONNECTED) ? String(WiFi.RSSI()) : "N/A";
    }
    
    static String getSSID() {
        return (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "нет подключения";
    }
};

class TimeManager {
private:
    TimeInfo info;
    
public:
    bool syncTime() {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        int attempts = 0;
        struct tm timeinfo;
        while (!getLocalTime(&timeinfo) && attempts < 10) {
            delay(500);
            attempts++;
        }
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Не удалось получить время");
            return false;
        }
        info.timeSynced = true;
        info.lastSync = millis();
        strftime(info.timeStr, sizeof(info.timeStr), "%H:%M:%S", &timeinfo);
        strftime(info.dateStr, sizeof(info.dateStr), "%d.%m.%Y", &timeinfo);
        Serial.print("Время синхронизировано: ");
        Serial.print(info.dateStr);
        Serial.print(" ");
        Serial.println(info.timeStr);
        return true;
    }
    
    void updateTime() {
        if (!info.timeSynced || (millis() - info.lastSync > 3600000)) {
            syncTime();
        } else {
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(info.timeStr, sizeof(info.timeStr), "%H:%M:%S", &timeinfo);
            strftime(info.dateStr, sizeof(info.dateStr), "%d.%m.%Y", &timeinfo);
        }
    }
    
    String getDateTime() {
        updateTime();
        return String(info.dateStr) + " " + String(info.timeStr);
    }
    
    String getTimestampForLog() {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", &timeinfo);
        return String(buffer);
    }
    
    TimeInfo* getInfo() { return &info; }
};

#endif