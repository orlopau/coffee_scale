#include <unity.h>

#include "embedded/constants.h"
#include "embedded/u8g_display.h"

U8GDisplay display(PIN_I2C_SDA, PIN_I2C_SCL);

void test_display_text(void)
{
    display.begin();
    display.display(1, 1000);
    Serial.parseInt();
}

void test_display_all()
{
    RUN_TEST(test_display_text);
}