#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "config.h"
#include "types.h"

class SensorManager {
private:
    SensorData data;
    Adafruit_AHTX0 aht;
    bool aht_initialized[8] = {false};

    void selectChannel(uint8_t channel) {
        if (channel > 7) return;
        Wire.beginTransmission(TCA9548A_ADDR);
        Wire.write(1 << channel);
        Wire.endTransmission();
        delay(1);
    }

public:
    void scanSensors() {
        Serial.println("Scanning sensors...");
        for (int channel = 0; channel < 8; channel++) {
            selectChannel(channel);
            if (aht.begin()) {
                data.present[channel] = true;
                aht_initialized[channel] = true;
                Serial.print("Sensor on channel ");
                Serial.println(channel + 1);
            } else {
                data.present[channel] = false;
                aht_initialized[channel] = false;
                Serial.print("No sensor on channel ");
                Serial.println(channel + 1);
            }
            delay(50);
        }
    }

    void readAllSensors() {
        for (int channel = 0; channel < 8; channel++) {
            if (aht_initialized[channel]) {
                selectChannel(channel);
                sensors_event_t t, h;
                if (aht.getEvent(&t, &h)) {
                    data.temperature[channel] = t.temperature;
                    data.humidity[channel] = h.relative_humidity;
                    data.present[channel] = true;
                } else {
                    data.present[channel] = false;
                }
            }
        }
    }

    SensorData* getData() { return &data; }
};

extern SensorManager sensorManager;
SensorManager sensorManager;

#endif