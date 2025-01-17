#ifndef NATIVE

#pragma once

// Pin definitions
#define PIN_BOOT 0

#define PIN_I2C_SDA 22
#define PIN_I2C_SCL 23

#define PIN_BUZZER 16

#define PIN_BAT_CHARGE_STATUS 32

#define PIN_HX711_SCK 25
#define PIN_HX711_DAT 26

#define PIN_ENC_A 18
#define PIN_ENC_B 19
#define PIN_ENC_BTN 17

#define PIN_BAT_ADC 36

// Other constants
#define BAT_V_MULTIPLIER 2.0f
#define BAT_V_MIN 3.2f
#define BAT_V_MAX 4.2f

#define ADC_OFFSET 0.04f

// EEPROM addresses
#define EEPROM_ADDR_SCALE 0
#define EEPROM_ADDR_FLOAT_SETTINGS 10

// BUTTONS
#define PIN_UPDATE_FIRMWARE PIN_ENC_BTN

// UPDATE
#define UPDATE_URL "https://github.com/orlopau/coffee-scale/releases/latest/download/firmware%s.bin"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0"
#endif

#endif
