#pragma once

#include "auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weight_sensor.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSensor &weightSensor, Stopwatch &stopwatch)
        : weightSensor(weightSensor), stopwatch(stopwatch){};
    ~ModeScale(){};
    void enter() override;
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    WeightSensor &weightSensor;
    Stopwatch &stopwatch;
    AutoTare *autoTare = new AutoTare(2, 1, 16); // 1g std deviation, 2g tolerance by default
};