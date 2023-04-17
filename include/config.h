#pragma once

/** USER CONFIGURABLE **/

/** General **/
#define ENABLE_INACTIVITY_ALERT false

/** Drive Characteristics **/
#define TURN_DAMPING_FACTOR 2

/** Wi-Fi **/
#define WIFI_SSID    ""
#define WIFI_PASS    ""
#define OTA_PASS     "FTB@dkh8pvk1cxa@ehb"
#define DEVICE_NAME  "roombii-roomba"
#define WIFI_AP_PASS "sucker"

/** NOT USER CONFIGURABLE **/

/** Tasks **/
#define TASK_PRIORITY_MOTOR tskIDLE_PRIORITY + 1
#define TASK_PRIORITY_STATE TASK_PRIORITY_MOTOR + 1
#define TASK_PRIORITY_LED   TASK_PRIORITY_WIFI + 1
#define TASK_PRIORITY_WIFI  TASK_PRIORITY_STATE + 1

/** General **/
#define FRONT_SPEAKER_CHANNEL     0
#define INACTIVITY_ALERT_TIMEOUT  300000 // 5 Minutes
#define INACTIVITY_ALERT_INTERVAL 60000  // 1 Minute
#define PARTY_LENGTH_MS           5000UL // 5 Seconds

/** VOLTAGE **/
#define VOLTAGE_DIVIDER_R1   100000 // 100kΩ
#define VOLTAGE_DIVIDER_R2   10000  // 10kΩ
#define VOLTAGE_NUM_READINGS 2

/** BUTTON **/
#define BUTTON_LONG_PRESS_TIME 2000
#define BUTTON_LONG_RESET_TIME 10000 // Reset button press parameters after this amount of time. Helps clear errors.

/* SPEED SENSING */
#define RIGHT_SPEED_SENSOR_PIN 32
#define LEFT_SPEED_SENSOR_PIN  14

/* MOTOR CONTROL */
// Indicates Adafruit Motor Controller slot.
#define RIGHT_DRIVE_MOTOR_ADDRESS 1
#define LEFT_DRIVE_MOTOR_ADDRESS  4
#define RIGHT_BRUSH_MOTOR_ADDRESS 2
#define LEFT_BRUSH_MOTOR_ADDRESS  3

/* CORE SENSORS */
#define RIGHT_LIFT_SENSOR_PIN 27
#define LEFT_LIFT_SENSOR_PIN  21
#define BIN_SENSOR_PIN        12
#define BUMP_SENSOR_PIN       13

/* VOLTAGE */
#define BATTERY_VOLTAGE_PIN                  A3
#define BATTERY_VOLTAGE_PIN_ADC_UNIT         ADC_UNIT_1
#define BATTERY_VOLTAGE_PIN_ADC_CHANNEL      ADC_CHANNEL_3
#define BATTERY_VOLTAGE_PIN_ADC_CHANNEL_FULL ADC1_CHANNEL_3

/* FRONT IO */
#define TOP_BUTTON_PIN    A5
#define FRONT_SPEAKER_PIN 5

/* NEOPIXELS */
#define NEOPIXELS_PIN    A1
#define NEOPIXELS_NUM    5
#define NEOPIXEL_WIFI_ID 4