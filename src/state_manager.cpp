#include "state_manager.h"

#define ALL_LED_SEGMENT_INDEX  2
#define WIFI_LED_SEGMENT_INDEX 0
#define BTN_LED_SEGMENT_INDEX  1

StateManager::StateManager(int right_lift_sensor_pin, int left_lift_sensor_pin, int bin_pin, int bump_pin) {
	_pin_bump              = bump_pin;
	_pin_bin               = bin_pin;
	_pin_right_lift_sensor = right_lift_sensor_pin;
	_pin_left_lift_sensor  = left_lift_sensor_pin;

	xStatusQueueHandle = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

	if (xStatusQueueHandle == NULL) {
		/* One or more queues were not created successfully as there was not enough
		heap memory available.  Handle the error here.  Queues can also be created
		statically. */
	}
}

void StateManager::begin() {
	pinMode(_pin_right_lift_sensor, INPUT);
	pinMode(_pin_left_lift_sensor, INPUT);
	pinMode(_pin_bin, INPUT);
	pinMode(_pin_bump, INPUT);

	_front_leds.init();
	_front_leds.setColor(BLACK);
	_determineMode();
	_front_leds.start();

	accept_user_drive_coordinates = xSemaphoreCreateMutex();

	if (accept_user_drive_coordinates == NULL) {
		Serial.println("Failed to create mutex");
	}

	// Task creation pattern: https://stackoverflow.com/questions/68979347/freertos-creating-a-task-within-a-c-class
	xTaskCreatePinnedToCore(
		this->_start_state_task_impl, /* Function to implement the task */
		"StateControl",               /* Name of the task */
		5000,                         /* Stack size in words */
		this,                         /* Task input parameter */
		TASK_PRIORITY_STATE,          /* Priority of the task */
		&_stateTaskHandle,            /* Task handle. */
		0);                           /* Core where the task should run */

	// Task creation pattern: https://stackoverflow.com/questions/68979347/freertos-creating-a-task-within-a-c-class
	xTaskCreatePinnedToCore(
		this->_start_led_task_impl, /* Function to implement the task */
		"LEDControl",               /* Name of the task */
		5000,                       /* Stack size in words */
		this,                       /* Task input parameter */
		TASK_PRIORITY_LED,          /* Priority of the task */
		&_ledTaskHandle,            /* Task handle. */
		0);                         /* Core where the task should run */
}

void StateManager::_start_state_task_impl(void *_this) {
	static_cast<StateManager *>(_this)->_state_task();
}

void StateManager::_start_led_task_impl(void *_this) {
	static_cast<StateManager *>(_this)->_led_task();
}

void StateManager::_state_task() {

	StateMessage xReceivedMessage;
	BaseType_t   notificationReceived;

	while (true) {
		if (xStatusQueueHandle != NULL) {

			notificationReceived = xQueueReceive(xStatusQueueHandle, &xReceivedMessage, portMAX_DELAY);

			if (notificationReceived == pdFALSE) {
				continue;
			}

			switch (xReceivedMessage.msg_subject) {
				case WIFI:
					_states.wifi = xReceivedMessage.state.wifi_state;
					break;
				case OTA:
					_states.ota = xReceivedMessage.state.ota_state;
					break;
				case OTA_PROGRESS:
					_states.ota_progress = xReceivedMessage.state.ota_progress;
					break;
				case BLUETOOTH:
					_states.bluetooth = xReceivedMessage.state.bluetooth_state;
					break;
				case FRONT_BUMP:
					_states.front_bumping = xReceivedMessage.state.front_bumping;
					break;
				case BIN_REMOVED:
					_states.bin_missing = xReceivedMessage.state.bin_missing;
					break;
				case LIFTED:
				case PARTY:
					_states.partying = true;
				case WIIMOTE_CONNECTED:
				case BATTERY:
					break;
			}
		}

		_determineMode();
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void StateManager::_led_task() {
	for (;;) {
		_front_leds.service();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

/**
 * SETTERS
 */

void StateManager::setOtaProgress(int progress) {
	StateMessage xMessage;
	xMessage.msg_subject        = OTA_PROGRESS;
	xMessage.state.ota_progress = progress;
	xQueueSend(xStatusQueueHandle, (void *)&xMessage, (TickType_t)0);
}

void StateManager::setWifiState(WifiState state) {
	StateMessage xMessage;
	xMessage.msg_subject      = WIFI;
	xMessage.state.wifi_state = state;
	xQueueSend(xStatusQueueHandle, (void *)&xMessage, (TickType_t)0);
}

void StateManager::setOtaState(OtaState state) {
	StateMessage xMessage;
	xMessage.msg_subject     = OTA;
	xMessage.state.ota_state = state;
	xQueueSend(xStatusQueueHandle, (void *)&xMessage, (TickType_t)0);
}

void StateManager::setBluetoothState(BluetoothState state) {
	StateMessage xMessage;
	xMessage.msg_subject           = BLUETOOTH;
	xMessage.state.bluetooth_state = state;
	xQueueSend(xStatusQueueHandle, (void *)&xMessage, (TickType_t)0);
}

void StateManager::startParty() {
	StateMessage xMessage;
	xMessage.msg_subject    = PARTY;
	xMessage.state.partying = true;
	xQueueSend(xStatusQueueHandle, (void *)&xMessage, (TickType_t)0);
}

/**
 * GETTERS
 */

int StateManager::getOtaProgress() {
	return _states.ota_progress;
}

WifiState StateManager::getWifiState() {
	return _states.wifi;
}
OtaState StateManager::getOtaState() {
	return _states.ota;
}

StateManager::BluetoothState StateManager::getBluetoothState() {
	return _states.bluetooth;
}

bool StateManager::isPartying() {
	return _states.partying;
}

/**
 * OTHER FUNCTIONS
 */

void StateManager::_determineMode() {

	Serial.println("[StateManager::_determineMode()] Determining state mode...");

	if (_states.ota > OTA_STATE_UPDATING) {
		// We never give this back. If OTA is successful, robot resets robot. If OTA fails, human resets robot.
		Serial.println("OTA_STATE_UPDATING Taking accept_user_drive_coordinates semaphore.");
		xSemaphoreTakeRecursive(accept_user_drive_coordinates, portMAX_DELAY);
	}

	if (_states.partying) {
		Serial.println("[StateManager::_determineMode()] PARTY TIME");
		Serial.println("PARTYING Taking accept_user_drive_coordinates semaphore.");
		xSemaphoreTakeRecursive(accept_user_drive_coordinates, portMAX_DELAY);
		_front_leds.resetSegments();
		_front_leds.setSegment(ALL_LED_SEGMENT_INDEX, 0, NEOPIXELS_NUM - 1, FX_MODE_RAINBOW_LARSON, WHITE, 1000);
		_front_leds.service(); // May not be necessary.
		vTaskDelay(PARTY_LENGTH_MS);
		_states.partying = false;
		xSemaphoreGiveRecursive(accept_user_drive_coordinates);
		_front_leds.resetSegments();
		_determineMode();
	} else {
		switch (_states.wifi) {
			case WIFI_STATE_DISCONNECTED:
			case WIFI_STATE_CONNECTING:
			case WIFI_STATE_GETTING_IP:
				_front_leds.setSegment(WIFI_LED_SEGMENT_INDEX, NEOPIXELS_NUM - 1, NEOPIXELS_NUM - 1, FX_MODE_BLINK, YELLOW, 750);
				break;
			case WIFI_STATE_CONNECTED:
				_front_leds.setSegment(WIFI_LED_SEGMENT_INDEX, NEOPIXELS_NUM - 1, NEOPIXELS_NUM - 1, FX_MODE_STATIC, GREEN, 0);
				break;
			case WIFI_STATE_ERROR:
				_front_leds.setSegment(WIFI_LED_SEGMENT_INDEX, NEOPIXELS_NUM - 1, NEOPIXELS_NUM - 1, FX_MODE_BLINK, RED, 750);
				break;
		}

		// Manage Button LEDs
		if (_states.bluetooth == BLUETOOTH_STATE_CONNECTED && (_states.ota == OTA_STATE_IDLE || _states.ota == OTA_STATE_SUCCESS)) {
			_front_leds.setSegment(BTN_LED_SEGMENT_INDEX, 0, NEOPIXELS_NUM - 2, FX_MODE_STATIC, GREEN, 1000);
		} else if (_states.ota == OTA_STATE_ERROR) {
			_front_leds.setSegment(BTN_LED_SEGMENT_INDEX, 0, NEOPIXELS_NUM - 2, FX_MODE_STATIC, RED, 1000);
		} else if (_states.bluetooth == BLUETOOTH_STATE_DISCONNECTED) {
			_front_leds.setSegment(BTN_LED_SEGMENT_INDEX, 0, NEOPIXELS_NUM - 2, FX_MODE_FADE, YELLOW, 250);
		}

		_front_leds.service();
	}
}
