#ifndef LOGGING_H
#define LOGGING_H

#include <SPIFFS.h>
#include "config.h"
#include "types.h"
#include "sensors.h"

extern unsigned long logStartTime;
extern String currentLogFile;
extern SensorManager sensorManager;

class LogManager {
private:
    static unsigned long measurementCounter;

    void writeFileHeader(File &file) {
        file.print("Начало измерений: ");
        file.print("(по времени ПК)");
        file.println();
        file.println();

        file.print("№\tdt");
        for (int i = 0; i < 8; i++) {
            file.print("\tКанал");
            file.print(i+1);
            file.print("_C");
            file.print("\tКанал");
            file.print(i+1);
            file.print("_%");
        }
        file.println();
        file.println();
        measurementCounter = 0;
    }

    void writeDataRow(File &file) {
        measurementCounter++;
        float dt = (millis() - logStartTime) / 1000.0;

        file.print(measurementCounter);
        file.print("\t");
        file.print(dt, 3);

        SensorData* data = sensorManager.getData();
        for (int i = 0; i < 8; i++) {
            file.print("\t");
            if (data->present[i]) {
                file.print(data->temperature[i], 2);
                file.print("\t");
                file.print(data->humidity[i], 2);
            } else {
                file.print("---\t---");
            }
        }
        file.println();
    }

    void writeFileFooter(File &file) {
        file.println();
        file.print("Конец измерений: ");
        file.println(millis() - logStartTime);
    }

public:
    void initSPIFFS() {
        // уже вызвано в setup
    }

    void listFilesSerial() {
        File root = SPIFFS.open("/");
        if (!root) return;
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (name.endsWith(".txt")) {
                Serial.print("FILE:");
                Serial.print(name);
                Serial.print(",");
                Serial.println(file.size());
            }
            file.close();
            file = root.openNextFile();
        }
        root.close();
        Serial.println("END_OF_LIST");
    }

    void cleanupOldLogs() {
        File root = SPIFFS.open("/");
        if (!root) return;
        File file = root.openNextFile();
        int fileCount = 0;
        String files[100];
        while (file && fileCount < 100) {
            String fileName = String(file.name());
            if (fileName.endsWith(".txt")) files[fileCount++] = fileName;
            file.close();
            file = root.openNextFile();
        }
        root.close();
        if (fileCount > 10) {
            for (int i = 0; i < fileCount-1; i++) {
                for (int j = i+1; j < fileCount; j++) {
                    if (files[i] > files[j]) {
                        String temp = files[i];
                        files[i] = files[j];
                        files[j] = temp;
                    }
                }
            }
            for (int i = 0; i < fileCount-10; i++) {
                if (SPIFFS.remove(files[i])) Serial.println("Deleted old log: " + files[i]);
            }
        }
    }

    void writeLogToFile() {
        if (currentLogFile == "") {
            currentLogFile = "/log_" + String(millis()) + ".txt";
            measurementCounter = 0;
        }
        File file = SPIFFS.open(currentLogFile, FILE_APPEND);
        if (!file) { Serial.println("Log file open error"); return; }
        if (file.size() == 0) writeFileHeader(file);
        writeDataRow(file);
        file.close();

        file = SPIFFS.open(currentLogFile, FILE_READ);
        if (file.size() > 100000) {
            file.close();
            finalizeLogFile();
            currentLogFile = "/log_" + String(millis()) + ".txt";
            measurementCounter = 0;
            Serial.println("New log file created (size >100KB)");
        } else {
            file.close();
        }
        Serial.println("Data written to " + currentLogFile);
    }

    void finalizeLogFile() {
        if (currentLogFile == "") return;
        File file = SPIFFS.open(currentLogFile, FILE_APPEND);
        if (file) {
            writeFileFooter(file);
            file.close();
            Serial.println("Log finished: " + currentLogFile);
        }
    }

    void clearAllLogs() {
        File root = SPIFFS.open("/");
        if (!root) return;
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            if (name.endsWith(".txt")) {
                SPIFFS.remove("/" + name);
                Serial.print("Deleted: "); Serial.println(name);
            }
            file.close();
            file = root.openNextFile();
        }
        root.close();
        currentLogFile = "";
        Serial.println("All logs deleted");
    }
};

unsigned long LogManager::measurementCounter = 0;
LogManager logManager;

#endif