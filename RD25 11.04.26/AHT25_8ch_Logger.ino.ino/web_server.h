#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_ntp.h"
#include "sensors.h"
#include "logging.h"
#include "html_page.h"

extern String formatInterval(unsigned long ms);

class WebServerManager {
private:
    WebServer* server;
    SensorManager* sensorManager;
    LogManager* logManager;
    TimeManager* timeManager;

    String formatTimeHMS(unsigned long ms) {
        unsigned long seconds = ms / 1000;
        unsigned long hours = seconds / 3600;
        unsigned long minutes = (seconds % 3600) / 60;
        unsigned long secs = seconds % 60;
        if (hours > 0) return String(hours) + ":" + (minutes<10?"0":"") + String(minutes) + ":" + (secs<10?"0":"") + String(secs);
        else if (minutes > 0) return String(minutes) + ":" + (secs<10?"0":"") + String(secs);
        else return String(secs) + " сек";
    }

    String generateLogFilesList() {
        String list = "";
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        int count = 0;
        while (file && count < 10) {
            if (String(file.name()).endsWith(".txt")) {
                String filename = String(file.name());
                if (filename.startsWith("/")) filename = filename.substring(1);
                list += "<div class='log-file-item'>";
                list += "<span class='log-file-name'>" + filename + "</span>";
                list += "<span class='log-file-size'>" + String(file.size()) + " байт</span>";
                list += "<div><button class='download-btn' onclick='downloadLogFile(\"" + filename + "\")'>Скачать</button>";
                list += "<button class='delete-btn' onclick='deleteLogFile(\"" + filename + "\")'>Удалить</button></div></div>";
                count++;
            }
            file.close();
            file = root.openNextFile();
        }
        root.close();
        return list.isEmpty() ? "<div>Файлы данных не найдены</div>" : list;
    }

public:
    WebServerManager(WebServer* srv, SensorManager* sm, LogManager* lm, TimeManager* tm)
        : server(srv), sensorManager(sm), logManager(lm), timeManager(tm) {
        setupRoutes();
    }

    void setupRoutes() {
        server->on("/", std::bind(&WebServerManager::handleRoot, this));
        server->on("/stopLogging", std::bind(&WebServerManager::handleStopLogging, this));
        server->on("/getLogRemaining", std::bind(&WebServerManager::handleGetLogRemaining, this));
        server->on("/data", std::bind(&WebServerManager::handleData, this));
        server->on("/reconnect", std::bind(&WebServerManager::handleReconnect, this));
        server->on("/scan", std::bind(&WebServerManager::handleScan, this));
        server->on("/syncTime", std::bind(&WebServerManager::handleSyncTime, this));
        server->on("/updateSettings", std::bind(&WebServerManager::handleUpdateSettings, this));
        server->on("/toggleLogging", std::bind(&WebServerManager::handleToggleLogging, this));
        server->on("/listLogs", std::bind(&WebServerManager::handleListLogs, this));
        server->on("/deleteLog", std::bind(&WebServerManager::handleDeleteLog, this));
        server->on("/downloadLogFile", std::bind(&WebServerManager::handleDownloadLogFile, this));
        server->on("/downloadCurrentLog", std::bind(&WebServerManager::handleDownloadCurrentLog, this));
        server->on("/clearLogs", std::bind(&WebServerManager::handleClearLogs, this));
    }

    void begin() {
        server->begin();
    }

    void handleRoot() {
        String page = FPSTR(htmlPage);
        page.replace("%WIFI_STATUS%", WiFiManager::getStatus());
        page.replace("%WIFI_STATUS_CLASS%", (WiFiManager::getStatus() == "ПОДКЛЮЧЕНО") ? "status-good" : "status-bad");
        page.replace("%WIFI_SSID%", WiFiManager::getSSID());
        page.replace("%IP%", WiFiManager::getIP());
        page.replace("%RSSI%", WiFiManager::getRSSI());

        Stats stats = sensorManager->calculateStats();
        page.replace("%ACTIVE_SENSORS%", String(stats.activeSensors));
        page.replace("%AVG_TEMP%", String(stats.avgTemp, 1));
        page.replace("%AVG_HUM%", String(stats.avgHum, 1));

        unsigned long pollingSeconds = pollingInterval / 1000;
        page.replace("%POLLING_HOURS%", String(pollingSeconds / 3600));
        page.replace("%POLLING_MINUTES%", String((pollingSeconds % 3600) / 60));
        page.replace("%POLLING_SECONDS%", String(pollingSeconds % 60));
        page.replace("%POLLING_RATE_FORMATTED%", formatInterval(pollingInterval));

        unsigned long intervalSeconds = logInterval / 1000;
        page.replace("%LOG_INTERVAL_HOURS%", String(intervalSeconds / 3600));
        page.replace("%LOG_INTERVAL_MINUTES%", String((intervalSeconds % 3600) / 60));
        page.replace("%LOG_INTERVAL_SECONDS%", String(intervalSeconds % 60));
        page.replace("%LOG_INTERVAL_FORMATTED%", formatInterval(logInterval));

        unsigned long durationSeconds = logDuration / 1000;
        page.replace("%LOG_DURATION_HOURS%", String(durationSeconds / 3600));
        page.replace("%LOG_DURATION_MINUTES%", String((durationSeconds % 3600) / 60));
        page.replace("%LOG_DURATION_SECONDS%", String(durationSeconds % 60));
        page.replace("%LOG_DURATION_FORMATTED%", logDuration == 0 ? "беск." : formatInterval(logDuration));

        String statusClass = "";
        String statusText = logManager->getLogStatusText();
        String countdownDisplay = "none";
        String countdownText = "";
        if (loggingEnabled && autoStopEnabled && logDuration > 0) {
            unsigned long remaining = logDuration - (millis() - logStartTime);
            if (remaining > 0) {
                statusClass = "log-countdown";
                countdownDisplay = "block";
                countdownText = formatTimeHMS(remaining);
            } else {
                statusClass = "log-completed";
            }
        } else if (loggingEnabled) {
            statusClass = "log-enabled";
        } else {
            statusClass = "log-disabled";
        }
        page.replace("%LOG_STATUS_CLASS%", statusClass);
        page.replace("%LOG_STATUS_TEXT%", statusText);
        page.replace("%COUNTDOWN_DISPLAY%", countdownDisplay);
        page.replace("%COUNTDOWN_TEXT%", countdownText);

        if (loggingEnabled) {
            page.replace("%LOG_BTN_CLASS%", "btn-danger");
            page.replace("%LOG_BTN_TEXT%", "Завершить запись");
        } else {
            page.replace("%LOG_BTN_CLASS%", "btn");
            page.replace("%LOG_BTN_TEXT%", "Запустить запись");
        }

        page.replace("%CURRENT_TIME%", timeManager->getDateTime());
        page.replace("%CURRENT_LOG_FILE%", logManager->getCurrentLogFile());
        page.replace("%SENSORS_GRID%", sensorManager->generateSensorsGrid());
        page.replace("%LAST_UPDATE%", timeManager->getDateTime());
        page.replace("%LOG_REMAINING_TIME%", logManager->getLogRemainingTime());
        page.replace("%LOG_FILES_LIST%", generateLogFilesList());

        server->send(200, "text/html", page);
    }

    void handleStopLogging() {
        if (loggingEnabled) {
            loggingEnabled = false;
            autoStopEnabled = false;
            logManager->finalizeLogFile();
            String json = "{\"success\":true,\"message\":\"Запись данных завершена\",\"time\":\"" + timeManager->getDateTime() + "\"}";
            server->send(200, "application/json", json);
        } else {
            server->send(200, "application/json", "{\"success\":false,\"error\":\"Запись данных не активна\"}");
        }
    }

    void handleGetLogRemaining() {
        server->send(200, "application/json", "{\"remaining\":" + logManager->getLogRemainingTime() + "}");
    }

    void handleData() {
        server->send(200, "application/json", sensorManager->getDataJSON());
    }

    void handleReconnect() {
        WiFi.disconnect();
        delay(1000);
        WiFiManager::connectWiFi();
        server->send(200, "text/plain", "OK");
    }

    void handleScan() {
        sensorManager->scanSensors();
        Stats stats = sensorManager->calculateStats();
        String json = "{\"found\":" + String(stats.activeSensors) + "}";
        server->send(200, "application/json", json);
    }

    void handleSyncTime() {
        if (timeManager->syncTime()) {
            String json = "{\"success\":true,\"time\":\"" + timeManager->getDateTime() + "\"}";
            server->send(200, "application/json", json);
        } else {
            server->send(200, "application/json", "{\"success\":false}");
        }
    }

    void handleUpdateSettings() {
        if (server->hasArg("polling")) {
            pollingInterval = server->arg("polling").toInt() * 1000;
            if (pollingInterval < 1000) pollingInterval = 1000;
            if (pollingInterval > 86400000) pollingInterval = 86400000;
        }
        String json = "{\"polling\":" + String(pollingInterval) + "}";
        server->send(200, "application/json", json);
    }

    void handleToggleLogging() {
        if (server->hasArg("interval")) {
            unsigned long newInterval = server->arg("interval").toInt() * 1000;
            if (newInterval == 0 || (newInterval >= MIN_LOG_INTERVAL && newInterval <= MAX_LOG_INTERVAL)) {
                logInterval = newInterval;
            }
        }
        if (server->hasArg("duration")) {
            logDuration = server->arg("duration").toInt() * 1000;
        }

        if (!loggingEnabled) {
            loggingEnabled = true;
            autoStopEnabled = (logDuration > 0);
            logStartTime = millis();
            currentLogFile = "";
            String json = "{\"status\":\"started\",\"interval\":" + String(logInterval) + ",\"duration\":" + String(logDuration) + "}";
            server->send(200, "application/json", json);
        } else {
            loggingEnabled = false;
            autoStopEnabled = false;
            logManager->finalizeLogFile();
            server->send(200, "application/json", "{\"status\":\"stopped\"}");
        }
    }

    void handleListLogs() {
        server->send(200, "application/json", logManager->listLogsJSON());
    }

    void handleDeleteLog() {
        if (!server->hasArg("file")) {
            server->send(200, "application/json", "{\"success\":false,\"error\":\"No file specified\"}");
            return;
        }
        String filename = "/" + server->arg("file");
        if (SPIFFS.exists(filename) && SPIFFS.remove(filename)) {
            server->send(200, "application/json", "{\"success\":true}");
        } else {
            server->send(200, "application/json", "{\"success\":false,\"error\":\"Cannot delete file\"}");
        }
    }

    void handleDownloadLogFile() {
        if (!server->hasArg("file")) {
            server->send(200, "text/plain", "Не указан файл");
            return;
        }
        String filename = "/" + server->arg("file");
        if (!SPIFFS.exists(filename)) {
            server->send(200, "text/plain", "Файл не найден");
            return;
        }
        File file = SPIFFS.open(filename, FILE_READ);
        if (!file) {
            server->send(200, "text/plain", "Ошибка открытия файла");
            return;
        }
        server->sendHeader("Content-Type", "text/plain; charset=utf-8");
        server->sendHeader("Content-Disposition", "attachment; filename=" + server->arg("file"));
        server->sendContent_P(PSTR("\xEF\xBB\xBF"));
        while (file.available()) {
            server->sendContent(file.readString());
        }
        file.close();
    }

    void handleDownloadCurrentLog() {
        String curFile = logManager->getCurrentLogFile();
        if (curFile == "" || !SPIFFS.exists(curFile)) {
            server->send(200, "text/plain", "Текущий файл данных не найден");
            return;
        }
        File file = SPIFFS.open(curFile, FILE_READ);
        if (!file) {
            server->send(200, "text/plain", "Ошибка открытия файла");
            return;
        }
        String downloadName = curFile.substring(1);
        server->sendHeader("Content-Type", "text/plain; charset=utf-8");
        server->sendHeader("Content-Disposition", "attachment; filename=" + downloadName);
        server->sendContent_P(PSTR("\xEF\xBB\xBF"));
        while (file.available()) {
            server->sendContent(file.readString());
        }
        file.close();
    }

    void handleClearLogs() {
        if (loggingEnabled) {
            loggingEnabled = false;
            autoStopEnabled = false;
            logManager->finalizeLogFile();
        }
        SPIFFS.end();
        delay(500);
        SPIFFS.begin(true);
        int deletedCount = 0;
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file) {
            String fileName = String(file.name());
            String fullPath = "/" + fileName;
            file.close();
            if (fileName.endsWith(".txt") && SPIFFS.remove(fullPath)) {
                deletedCount++;
            }
            file = root.openNextFile();
        }
        root.close();
        currentLogFile = "";
        server->send(200, "application/json", "{\"success\":true,\"deleted\":" + String(deletedCount) + "}");
    }
};

#endif