#include "mode_manager.h"
#include "millis.h"
#include "battery.h"
#include "interface.h"

ModeManager::ModeManager(Mode *modes[], const int modeCount, Display &display)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), display(display), lastBatteryTime(0)
{
}

void ModeManager::update()
{
    if (inModeChange)
    {
        currentMode += static_cast<int>(Interface::getEncoderDirection());

        if (currentMode < 0)
        {
            currentMode = 0;
        }
        else if (currentMode >= modeCount)
        {
            currentMode = modeCount - 1;
        }

        if (now() > lastBatteryTime + BATTERY_UPDATE_INTERVAL || lastBatteryTime == 0)
        {
            lastBatteryTime = now();
            lastVoltage = Battery::getVoltage();
            lastPercentage = Battery::getPercentage();
        }

        display.modeSwitcher(modes[currentMode]->getName(), currentMode, modeCount, lastVoltage, lastPercentage, Battery::isCharging());

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            inModeChange = false;
            modes[currentMode]->enter();
        }
    }
    else
    {
        if (Interface::getEncoderClick() == ClickType::LONG && modes[currentMode]->canSwitchMode())
        {
            inModeChange = true;
        }
        else
        {
            modes[currentMode]->update();
        }
    }
}