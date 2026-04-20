#ifndef TYPES_H
#define TYPES_H

struct SensorData {
    float temperature[8] = {0};
    float humidity[8] = {0};
    bool present[8] = {false};
};

struct Stats {
    int activeSensors;
    float avgTemp;
    float avgHum;
};

#endif