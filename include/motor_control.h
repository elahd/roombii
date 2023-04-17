#pragma once

#include "config.h"
#include "pitches.h"

#include <Adafruit_MotorShield.h>
#include <Arduino.h>
#include <Bluepad32.h>
#include <Wire.h>
#include <math.h>

const int8_t ACTION_NONE  = 0;
const int8_t ACTION_PARTY = 1;
const int8_t ACTION_BRAKE = 2;

class MotorControl {
  public:
	MotorControl(int leftMotorPin, int rightMotorPin, int leftBrushPin, int rightBrushPin, int rightSpeedSensorPin, int leftSpeedSensorPin, int sdaPin = SDA, int sclPin = SCL);
	void brake();
	void begin();

	void brush(bool brush);
	void twirl();
	void motor_control(int32_t x, int32_t y, bool brush_btn);

  private:
	void                 _drive_from_joystick_coords(int32_t x, int32_t y, bool brush = false);
	Adafruit_MotorShield _AFMS;
	Adafruit_DCMotor    *_rightMotor;
	Adafruit_DCMotor    *_leftMotor;
	Adafruit_DCMotor    *_rightBrush;
	Adafruit_DCMotor    *_leftBrush;
	void                 _set_motor(Adafruit_DCMotor *motor, int throttle);
	void                 _convert_xy_to_throttles(int32_t x, int32_t y, int32_t coords[2]);

	int           _rightSpeedSensorPin;
	int           _leftSpeedSensorPin;
	int           _sdaPin;
	int           _sclPin;
	int           _leftThrottle;
	int           _rightThrottle;
	void          send_drive_action(uint8_t action);
	unsigned long _last_command;

	static void  _start_motor_task_impl(void *_this);
	void         _motor_task();
	TaskHandle_t _driveTaskHandle;

	typedef union {
		uint32_t bits; // Bit representation of the notification
		struct {
			int8_t brush_spd;       // -255 to 255
			int8_t wheel_right_spd; // -255 to 255
			int8_t wheel_left_spd;  // -255 to 255
			int8_t actions;         // ACTION_NONE, ACTION_PARTY, ACTION_BRAKE
		} fields;
	} motor_notification_t;
};