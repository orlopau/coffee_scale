#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"
#include "formatters.h"
#include "millis.h"

#include <Arduino.h>

void ModeDefault::update()
{
    display.display(loadCell.getWeight(), stopwatch.getTime());

    if (buttons.getEncoderClick() == ClickType::LONG)
    {
        loadCell.tare();
    }

    if (buttons.getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
    }
}