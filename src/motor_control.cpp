#include "motor_control.h"

MotorControl::MotorControl(int leftMotorPin, int rightMotorPin, int leftBrushPin, int rightBrushPin, int rightSpeedSensorPin, int leftSpeedSensorPin, int sdaPin, int sclPin) {
	_AFMS                = Adafruit_MotorShield();
	_rightMotor          = _AFMS.getMotor(rightMotorPin);
	_leftMotor           = _AFMS.getMotor(leftMotorPin);
	_rightBrush          = _AFMS.getMotor(rightBrushPin);
	_leftBrush           = _AFMS.getMotor(leftBrushPin);
	_rightSpeedSensorPin = rightSpeedSensorPin;
	_leftSpeedSensorPin  = leftSpeedSensorPin;
	_sdaPin              = sdaPin;
	_sclPin              = sclPin;
	_last_command        = 0;
}

/**
 * @brief Converts gamepad joystick input to left and right wheel throttle.
 *
 * @param x X-axis position between -256 and 256
 * @param y Y-axis position between -256 and 256.
 * @return int* Array of two integers, left and right throttle, in the order [left, right].
 */
void MotorControl::_convert_xy_to_throttles(int32_t x, int32_t y, int32_t coords[2]) {
	// Reference:
	// https://robotics.stackexchange.com/questions/20347/how-do-i-convert-centre-returning-joystick-values-to-dual-hobby-motor-direction

	y = -y;
	x = -x;

	if (x == 0 && y == 0) {
		coords[0] = 0;
		coords[1] = 0;
		return;
	}

	// Convert to polar coordinates
	float theta = atan2(y, x);         // angle
	float r     = sqrt(x * x + y * y); // distance

	// Calculate maximum r for given theta
	float r_max;
	if (abs(x) > abs(y)) {
		r_max = abs(r / x);
	} else {
		r_max = abs(r / y);
	}

	// Actual throttle
	float magnitude = r / r_max;

	// Set wheel velocities
	coords[0] = magnitude * (sin(theta) + cos(theta) / TURN_DAMPING_FACTOR); // left
	coords[1] = magnitude * (sin(theta) - cos(theta) / TURN_DAMPING_FACTOR); // right

	// Serial.printf("[MotorControl:_convert_xy_to_throttles] left: %d, right: %d\n", coords[0], coords[1]);

	return;
}

/**
 * @brief Directly controls wheel and brush motors using joystick coordinates input.
 *
 * @param x X-axis position between -256 and 256
 * @param y Y-axis position between -256 and 256.
 * @param brush Whether brushes should run while driving.
 */
void MotorControl::_drive_from_joystick_coords(int32_t x, int32_t y, bool brush) {
	int32_t throttles[2];
	_convert_xy_to_throttles(x, y, throttles);

	// Send throttle to motors.
	_set_motor(_leftMotor, throttles[0]);
	_set_motor(_rightMotor, throttles[1]);

	if (brush) {
		_set_motor(_leftBrush, 255);
		_set_motor(_rightBrush, 255);
	} else {
		_set_motor(_leftBrush, 0);
		_set_motor(_rightBrush, 0);
	}
}

/**
 * @brief Drives individual motor at a given speed.
 *
 * @param motor Adafruit motor object.
 * @param throttle Throttle value between -255 and 255.
 */
void MotorControl::_set_motor(Adafruit_DCMotor *motor, int throttle) {

	int absThrottle = abs(throttle) > 255 ? 255 : abs(throttle);

	if (throttle > 0) {
		motor->setSpeed(absThrottle);
		motor->run(FORWARD);
	} else if (throttle < 0) {
		motor->setSpeed(absThrottle);
		motor->run(BACKWARD);
	} else {
		motor->run(RELEASE);
	}
}

/**
 * @brief Task for controlling motors. Received commands via FreeRTOS task notifications.
 *
 * the 32-bit notification value has the following format:
 *
 *  [brushes] [wheel_left] [wheel_right] [actions]
 *  31     24 23        16 15          8 7       4 3      0
 *                                                 [unused]
 *
 * - Wheel values are between -255 and 255. Wheel values should represent throttle values, not joystick coordinates.
 * - Brush values are between -128 and 128 (4 bits). These will ultimately be converted to 8 bit values.
 *   This is to conserve space as notification values can't exceed 32 bits. We don't need fine-grained resolution on brush speeds.
 */
void MotorControl::_motor_task(void) {
	Wire.setTimeout(10);

	if (!_AFMS.begin(1000)) {
		Serial.println("Could not find Motor Shield. Check wiring.");
		while (1) {
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
	}
	Serial.println("Motor Shield found.");

	// Set the speed to start, from 0 (off) to 255 (max speed)
	_rightMotor->setSpeed(0);
	_rightMotor->run(RELEASE);

	_leftMotor->setSpeed(0);
	_leftMotor->run(RELEASE);

	_rightBrush->setSpeed(0);
	_rightBrush->run(RELEASE);

	_leftBrush->setSpeed(0);
	_leftBrush->run(RELEASE);

	Serial.println("Initialized motor control. Starting task.");

	uint32_t   ulNotificationValue;
	BaseType_t notificationReceived;

	for (;;) {

		notificationReceived = xTaskNotifyWait(0x00,                 /* Don't clear any bits on entry. */
											   ULONG_MAX,            /* Clear all bits on exit. */
											   &ulNotificationValue, /* Receives the notification value. */
											   portMAX_DELAY);       /* Block indefinitely. */

		if (notificationReceived == pdFALSE) {
			_leftMotor->run(RELEASE);
			_rightMotor->run(RELEASE);
			_leftBrush->run(RELEASE);
			_rightBrush->run(RELEASE);
			continue;
		}

		motor_notification_t incoming_notification = {.bits = ulNotificationValue};

		int8_t brush       = incoming_notification.fields.brush_spd;
		int8_t wheel_left  = incoming_notification.fields.wheel_left_spd;
		int8_t wheel_right = incoming_notification.fields.wheel_right_spd;
		int8_t actions     = incoming_notification.fields.actions;

		// Serial.printf("[MotorControl::_motor_task] Received notification: brush=%d, wheel_left=%d, wheel_right=%d, actions=%d", brush, wheel_left, wheel_right, actions);

		if (actions == ACTION_PARTY) {
			// party mode
			Serial.println("[MotorControl::_motor_task] Party mode!");
			delay(100);

			for (int i = 0; i < 3; i++) {
				_drive_from_joystick_coords(255, 0);
				delay(500);
				_drive_from_joystick_coords(-255, 0);
				delay(500);
			}
			_drive_from_joystick_coords(0, 0);
			delay(100);
		} else if (actions == ACTION_BRAKE) {
			_leftMotor->run(BRAKE);
			_rightMotor->run(BRAKE);
			_leftBrush->run(BRAKE);
			_rightBrush->run(BRAKE);
		} else {
			// normal mode
			_set_motor(_leftMotor, wheel_left * 2);
			_set_motor(_rightMotor, wheel_right * 2);
			_set_motor(_leftBrush, brush * 2);
			_set_motor(_rightBrush, brush * 2);
		}

		vTaskDelay(10 / portTICK_PERIOD_MS); // Was 50. Change back if crashing.
	}
}

/**
 * @brief Starts _motor_task. This step is necessary for starting a task that lives within a class.
 */
void MotorControl::_start_motor_task_impl(void *_this) {
	static_cast<MotorControl *>(_this)->_motor_task();
}

void MotorControl::begin(void) {

	// Task creation pattern: https://stackoverflow.com/questions/68979347/freertos-creating-a-task-within-a-c-class
	xTaskCreatePinnedToCore(
		this->_start_motor_task_impl, /* Function to implement the task */
		"MotorControl",               /* Name of the task */
		5000,                         /* Stack size in words */
		this,                         /* Task input parameter */
		TASK_PRIORITY_MOTOR,          /* Priority of the task */
		&_driveTaskHandle,            /* Task handle. */
		0);                           /* Core where the task should run */
}

/*
@brief: Drive the robot from joystick coordinates.
@param x: The x coordinate of the joystick, between -512 and 512.
@param y: The y coordinate of the joystick, between -512 and 512.
@param brush: Whether the brush button has been pressed.
*/
void MotorControl::motor_control(int32_t x, int32_t y, bool brush_btn) {

	// Serial.printf("[MotorControl::motor_control] x=%d, y=%d, brush_btn=%d\n", x, y, brush_btn);

	int32_t throttles[2];
	_convert_xy_to_throttles(x, y, throttles); // Scale down -512-512 to -256-256
	int8_t actions = ACTION_NONE;

	motor_notification_t notification;
	notification.fields.brush_spd       = brush_btn || throttles[0] || throttles[1] ? 127 : 0;
	notification.fields.wheel_left_spd  = throttles[0] == 0 ? 0 : throttles[0] / 4;
	notification.fields.wheel_right_spd = throttles[1] == 0 ? 0 : throttles[1] / 4;
	notification.fields.actions         = actions;

	// Serial.printf("[MotorControl::motor_control] brush=%d, wheel_left=%d, wheel_right=%d, actions=%d, notification=%d\n", notification.fields.brush_spd, notification.fields.wheel_left_spd, notification.fields.wheel_right_spd, notification.fields.actions, notification.bits);

	xTaskNotify(this->_driveTaskHandle,
				notification.bits,
				eSetValueWithOverwrite);
}

void MotorControl::send_drive_action(uint8_t action) {

	motor_notification_t notification;
	notification.fields.actions = action;

	xTaskNotify(this->_driveTaskHandle,
				notification.bits,
				eSetValueWithOverwrite);
}

void MotorControl::twirl(void) {
	this->send_drive_action(ACTION_PARTY);
}

void MotorControl::brake(void) {
	this->send_drive_action(ACTION_BRAKE);
}
