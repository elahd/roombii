#pragma once

#include "config.h"

#include <Arduino.h>
// #include <jled.h>

/**
 * BUTTON LED
 */

// OTA In Progress
// JLed button_led_program_mode_p[] = {
// 	JLed(FRONT_RED_LED_PIN).Blink(1000, 500).DelayBefore(500).Forever(),
// 	JLed(FRONT_GREEN_LED_PIN).Blink(1000, 500).Forever()};
// auto button_led_program_mode_s = JLedSequence(JLedSequence::eMode::PARALLEL, button_led_program_mode_p).Forever();

// // OK
// JLed button_led_ok_p[] = {
// 	JLed(FRONT_RED_LED_PIN).Off(),
// 	JLed(FRONT_GREEN_LED_PIN).On()};
// auto button_led_ok_s = JLedSequence(JLedSequence::eMode::PARALLEL, button_led_ok_p).Forever();

// JLed button_led_btsearch_p[] = {
// 	JLed(FRONT_RED_LED_PIN).Off(),
// 	JLed(FRONT_GREEN_LED_PIN).Blink(1000, 200).Forever()};
// auto button_led_btsearch_s = JLedSequence(JLedSequence::eMode::PARALLEL, button_led_btsearch_p).Forever();

/**
 * WIFI LED
 */

// auto led_wifi_connecting   = JLed(FRONT_BLUE_LED_PIN).Blink(1000, 200).Forever();
// auto led_wifi_disconnected = JLed(FRONT_BLUE_LED_PIN).Breathe(1000).Forever();
// auto led_wifi_ok           = JLed(FRONT_BLUE_LED_PIN).On();