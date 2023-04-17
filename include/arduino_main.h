#pragma once

#include "sdkconfig.h"
#ifndef CONFIG_BLUEPAD32_PLATFORM_ARDUINO
#	error "Must only be compiled when using Bluepad32 Arduino platform"
#endif // !CONFIG_BLUEPAD32_PLATFORM_ARDUINO
#include "config.h"
#include "led_states.h"
#include "motor_control.h"
#include "pitches.h"
#include "state_manager.h"
#include "wifi_manager.h"

#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <Bluepad32.h>
#include <OneButton.h>
#include <esp_adc_cal.h>
#include <esp_log.h>


// static const char *TAG = "global";

/**
 * GLOBALS
 */

extern bool                           program_mode;
extern float                          battery_voltage_mv;
extern esp_adc_cal_characteristics_t *adc_chars;

/**
 * FUNCTIONS
 */

/** ARDUINO_MAIN.CPP **/

extern void enter_program_mode();
extern void processWebSerial(String rcv_data);
extern void readBatteryLevel(TimerHandle_t xTimer);
void        wifi_state_cb(WifiState state, String msg);
void        ota_state_cb(OtaState state, String msg, int pct);