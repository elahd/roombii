
#include "arduino_main.h"

/**
 * VARIABLES
 */

// /** STATE **/
// static StaticQueue_t xStatusQueue;

/** INTERRUPTS **/
volatile bool bumping = false;
portMUX_TYPE  mux     = portMUX_INITIALIZER_UNLOCKED;

/** OVERHEAD **/
TimerHandle_t batteryRefreshTimer;
unsigned long last_user_input       = 0UL;
unsigned long last_inactivity_alert = 0UL;

/** VOLTAGE **/
float bus_voltage;
float shunt_voltage;
float current_mA;
float total_mAH;
// Keep track of total time and milliamp measurements for milliamp-hour computation.
uint32_t        total_sec = 0;
float           total_mA  = 0.0;
Adafruit_INA219 ina219;

/** WI-FI **/
WifiManager mgr_wifi((const char *)WIFI_SSID, (const char *)WIFI_PASS, (const char *)DEVICE_NAME, (const char *)OTA_PASS, [](WifiState state, String msg) { wifi_state_cb(state, msg); }, [](OtaState state, String msg, int pct) { ota_state_cb(state, msg, pct); });

/** MOTOR CONTROL **/
MotorControl motorController = MotorControl(
	LEFT_DRIVE_MOTOR_ADDRESS, RIGHT_DRIVE_MOTOR_ADDRESS, LEFT_BRUSH_MOTOR_ADDRESS, RIGHT_BRUSH_MOTOR_ADDRESS, LEFT_SPEED_SENSOR_PIN, RIGHT_SPEED_SENSOR_PIN, SDA, SCL);

bool program_mode = false;

/** BUTTON **/
OneButton top_button = OneButton(
	TOP_BUTTON_PIN, // Input pin for the button
	false,          // Button is active LOW
	false           // Enable internal pull-up resistor
);

/** WIIMOTE **/
GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

/** STATE MANAGER **/
StateManager mgr_state = StateManager(RIGHT_LIFT_SENSOR_PIN, LEFT_LIFT_SENSOR_PIN, BIN_SENSOR_PIN, BUMP_SENSOR_PIN);

/**************/
/** FUNCTIONS */
/**************/

/**
 * WI-FI FUNCTIONS
 */
void wifi_state_cb(WifiState state, String msg = "") {
	mgr_state.setWifiState(state);
}

void ota_state_cb(OtaState state, String msg = "", int pct = 0) {
	mgr_state.setOtaState(state);
	mgr_state.setOtaProgress(pct);
}

/**
 * BUTTON FUNCTIONS
 */

void IRAM_ATTR checkTicks() {
	// include all buttons here to be checked
	top_button.tick(); // just call tick() to check the state.
}

static void handleClick() {
	// Short press; dance!
	mgr_state.startParty();
	tone(FRONT_SPEAKER_PIN, NOTE_G5, 250);
	tone(FRONT_SPEAKER_PIN, NOTE_G5, 250);
	motorController.twirl();
}

static void handleLongPressStart() {
	tone(FRONT_SPEAKER_PIN, NOTE_A4, 100);
	tone(FRONT_SPEAKER_PIN, NOTE_B4, 100);
	// button_led_program_mode_s.Update(); JLED
}

static void handleLongPressStop() {
	program_mode = true;
	// enter_program_mode();
}

/**
 * INTERRUPT FUNCTIONS
 */

void IRAM_ATTR isr_bumped() {
	portENTER_CRITICAL_ISR(&mux);
	bumping = true;
	portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR isr_unbumped() {
	portENTER_CRITICAL_ISR(&mux);
	bumping = false;
	portEXIT_CRITICAL_ISR(&mux);
}

void readBatteryLevel(TimerHandle_t xTimer) {
	// 1142mv == 11.74v. 1142 should be 12.562

	Serial.println("Reading battery level...");

	shunt_voltage = ina219.getShuntVoltage_mV();
	bus_voltage   = ina219.getBusVoltage_V();
	current_mA    = ina219.getCurrent_mA();

	// Compute load voltage, power, and milliamp-hours.
	float load_voltage = bus_voltage + (shunt_voltage / 1000);
	float power_mW     = load_voltage * current_mA;
	(void)power_mW;

	total_sec += 1;
	float total_mAH = total_mA / 3600.0;
	(void)total_mAH;

	// Serial.println("Printing...");

	Serial.printf("Shunt: %f mV, Bus: %f V, Current: %f mA, Power: %f mW, mAH: %f\n", shunt_voltage, bus_voltage, current_mA, power_mW, total_mAH);
}

/**
 * MODES
 */

void update_bt_controller_status() {
	int num_gamepads = 0;
	for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
		GamepadPtr myGamepad = myGamepads[i];

		if (myGamepad && myGamepad->isConnected()) {
			num_gamepads++;
		}
	}
	if (num_gamepads == 0) {
		mgr_state.setBluetoothState(StateManager::BLUETOOTH_STATE_DISCONNECTED);
	} else {
		mgr_state.setBluetoothState(StateManager::BLUETOOTH_STATE_CONNECTED);
	}
}

void onConnectedGamepad(GamepadPtr gp) {
	bool foundEmptySlot = false;
	for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
		if (myGamepads[i] == nullptr) {
			tone(FRONT_SPEAKER_PIN, NOTE_C7, 200);
			tone(FRONT_SPEAKER_PIN, NOTE_G7, 200);
			Serial.printf("CALLBACK: Gamepad is connected, index=%d\n", i);
			// Additionally, you can get certain gamepad properties like:
			// Model, VID, PID, BTAddr, flags, etc.
			GamepadProperties properties = gp->getProperties();
			Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n", gp->getModelName().c_str(), properties.vendor_id,
						  properties.product_id);

			myGamepads[i]  = gp;
			foundEmptySlot = true;
			break;
		}
	}
	if (!foundEmptySlot) {
		Serial.println("CALLBACK: Gamepad connected, but could not found empty slot");
	}
	update_bt_controller_status();
}

void onDisconnectedGamepad(GamepadPtr gp) {
	bool foundGamepad = false;

	for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
		if (myGamepads[i] == gp) {
			tone(FRONT_SPEAKER_PIN, NOTE_G7, 200);
			tone(FRONT_SPEAKER_PIN, NOTE_C7, 200);
			Serial.printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
			myGamepads[i] = nullptr;
			foundGamepad  = true;
			break;
		}
	}

	if (!foundGamepad) {
		Serial.println("CALLBACK: Gamepad disconnected, but not found in myGamepads");
	}
	update_bt_controller_status();
}

void setup() {
	Serial.begin(115200);

	// Suppress "WiFi" messages
	esp_log_level_set("wifi", ESP_LOG_WARN);

	// Initialize NeoPixels
	mgr_state.begin();

	// Initialize buttons.
	top_button.attachClick(handleClick);
	top_button.attachLongPressStart(handleLongPressStart);
	top_button.attachLongPressStop(handleLongPressStop);
	top_button.setPressTicks(BUTTON_LONG_PRESS_TIME);
	attachInterrupt(digitalPinToInterrupt(TOP_BUTTON_PIN), checkTicks, CHANGE);

	// Initialize sensors.
	pinMode(BUMP_SENSOR_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(BUMP_SENSOR_PIN), isr_bumped, FALLING);

	motorController.begin();

	batteryRefreshTimer = xTimerCreate("Battery Refresh Timer",
									   pdMS_TO_TICKS(60000),
									   pdTRUE,
									   (void *)0,
									   readBatteryLevel);
	xTimerStart(batteryRefreshTimer, 0);

	// Initialize the INA219
	if (!ina219.begin()) {
		Serial.println("Failed to find INA219 chip");
		while (1) {
			delay(10);
		}
	}

	ina219.setCalibration_16V_400mA();

	// readBatteryLevel(batteryRefreshTimer);
	/** Check Voltage: End **/

	Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

	// Setup Bluepad32
	BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
	// BP32.forgetBluetoothKeys();

	last_user_input = millis();

	// Initialize Wi-Fi
	mgr_wifi.begin();
}

// Arduino loop function. Runs in CPU 1
void loop() {

	delay(50);

#if ENABLE_INACTIVITY_ALERT
	if (((millis() - last_user_input) > INACTIVITY_ALERT_TIMEOUT) && ((millis() - last_inactivity_alert) > INACTIVITY_ALERT_INTERVAL)) {
		last_inactivity_alert = millis();
		tone(FRONT_SPEAKER_PIN, NOTE_C5, 100);
		delay(100);
		tone(FRONT_SPEAKER_PIN, NOTE_C5, 100);
		delay(100);
		tone(FRONT_SPEAKER_PIN, NOTE_C5, 100);
		delay(100);
		tone(FRONT_SPEAKER_PIN, NOTE_C5, 100);
		delay(100);
		tone(FRONT_SPEAKER_PIN, NOTE_C5, 100);
	}
#endif

	BaseType_t receivedSemaphore = xSemaphoreTake(mgr_state.accept_user_drive_coordinates, portMAX_DELAY);

	if (receivedSemaphore == pdFALSE) {
		Serial.println("[LOOP] Timed out waiting for semaphore.");
		return;
	}

	BP32.update();
	top_button.tick();

	for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
		GamepadPtr myGamepad = myGamepads[i];

		if (myGamepad && myGamepad->isConnected()) {

			// -512 to 512
			int32_t x_pos = (-5 < myGamepad->axisX() && myGamepad->axisX() < 5) ? 0 : myGamepad->axisX();
			int32_t y_pos = (-5 < myGamepad->axisY() && myGamepad->axisY() < 5) ? 0 : myGamepad->axisY() * -1;

			// Serial.println("[Main Loop] Controller X: " + String(x_pos) + " Controller Y: " + String(y_pos));

			motorController.motor_control(x_pos, y_pos, myGamepad->b());

			if ((x_pos != 0 || y_pos != 0) && !myGamepad->b()) {
				last_user_input = millis();
			}
		}
	}

	xSemaphoreGive(mgr_state.accept_user_drive_coordinates);
}