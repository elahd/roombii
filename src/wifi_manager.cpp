#include "wifi_manager.h"

/**
 * WIFI CONNECTION
 */

// TODO: Wi-Fi doesn't reconnect when connection is lost.

// Using lambdas in WiFi.onEvent and WebSerial.msgCallback to work around issues with using these functions within classes.
// See https://www.appsloveworld.com/cplus/100/274/refactoring-sample-code-into-class-raises-no-instance-of-overloaded-function

void WifiManager::_wifiConnectingCallback(arduino_event_id_t event, arduino_event_info_t info) {
	String msg = "Connecting to Wi-Fi...";
	Serial.println(msg);
	_wifi_state_cb(WIFI_STATE_CONNECTING, msg);
}

void WifiManager::_wifiConnectedCallback(arduino_event_id_t event, arduino_event_info_t info) {
	String msg = "Wi-Fi Connected! Getting IP...";
	Serial.println(msg);
	_wifi_state_cb(WIFI_STATE_GETTING_IP, msg);
}

void WifiManager::_wifiDisconnectedCallback(arduino_event_id_t event, arduino_event_info_t info) {
	String msg = "Wi-Fi Disconnected!";
	Serial.printf("%s (Reason: %i)\n", msg.c_str(), info.wifi_sta_disconnected.reason);

	_wifi_state_cb(WIFI_STATE_DISCONNECTED, msg);
}

void WifiManager::_wifiGotIpCallback(arduino_event_id_t event, arduino_event_info_t info) {
	Serial.println("Got IP address.");

	_wifi_state_cb(WIFI_STATE_CONNECTED, "");

	Serial.print("IP address: ");
	String ip_addr = WiFi.localIP().toString().c_str();
	Serial.println(ip_addr);

	ArduinoOTA.begin();

	_ota_state_cb(OTA_STATE_IDLE, "", 0);
}

void WifiManager::_wifiLostIpCallback(arduino_event_id_t event, arduino_event_info_t info) {
	Serial.println("Lost IP address.");

	_wifi_state_cb(WIFI_STATE_GETTING_IP, "");

	ArduinoOTA.end();

	_ota_state_cb(OTA_STATE_IDLE, "", 0);
}

WifiManager::WifiManager(const char *wifi_client_ssid, const char *wifi_client_pass, const char *device_name, const char *ota_pass, WifiStateCallback wifi_state_cb, OtaStateCallback ota_state_cb) {
	_wifi_client_ssid = wifi_client_ssid;
	_wifi_client_pass = wifi_client_pass;
	_ota_hostname     = device_name;
	_ota_pass         = ota_pass;
	_wifi_state_cb    = wifi_state_cb;
	_ota_state_cb     = ota_state_cb;

	// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino
	WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->_wifiConnectedCallback(event, info); }, ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->_wifiDisconnectedCallback(event, info); }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
	WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->_wifiGotIpCallback(event, info); }, ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->_wifiLostIpCallback(event, info); }, ARDUINO_EVENT_WIFI_STA_LOST_IP);
	WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) { this->_wifiConnectingCallback(event, info); }, ARDUINO_EVENT_WIFI_STA_START);
}

void WifiManager::begin() {
	// Task creation pattern: https://stackoverflow.com/questions/68979347/freertos-creating-a-task-within-a-c-class
	xTaskCreatePinnedToCore(
		this->_start_wifi_task_impl, /* Function to implement the task */
		"MotorControl",              /* Name of the task */
		5000,                        /* Stack size in words */
		this,                        /* Task input parameter */
		TASK_PRIORITY_WIFI,          /* Priority of the task */
		&_wifiTaskHandle,            /* Task handle. */
		0);                          /* Core where the task should run */
}

void WifiManager::_start_wifi_task_impl(void *_this) {
	static_cast<WifiManager *>(_this)->_wifi_task();
}

void WifiManager::_wifi_task(void) {
	Serial.println("Initializing Wi-Fi...");

	WiFi.mode(WIFI_STA);
	esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);

	Serial.println("Connecting to Wi-Fi...");
	WiFi.begin(_wifi_client_ssid, _wifi_client_pass);

	Serial.println("Starting Arduino OTA Server...");

	// & is used to pass a reference to *this* WifiManager to the lambda function, enabling access to _ota_state_cb.
	ArduinoOTA
		.onStart([&]() {
			_ota_state_cb(OTA_STATE_UPDATING, "", 0);
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("Start updating " + type);
		})
		.onEnd([&]() {
			_ota_state_cb(OTA_STATE_SUCCESS, "", 0);
			Serial.println("\nEnd");
		})
		.onProgress([&](unsigned int progress, unsigned int total) {
			int prog_pct = progress / (total / 100);
			_ota_state_cb(OTA_STATE_UPDATING, "", prog_pct);
			Serial.printf("Progress: %u%%\r", prog_pct);
		})
		.onError([&](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			String msg = "";
			if (error == OTA_AUTH_ERROR)
				msg = "Auth Failed";
			else if (error == OTA_BEGIN_ERROR)
				msg = "Begin Failed";
			else if (error == OTA_CONNECT_ERROR)
				msg = "Connect Failed";
			else if (error == OTA_RECEIVE_ERROR)
				msg = "Receive Failed";
			else if (error == OTA_END_ERROR)
				msg = "End Failed";
			_ota_state_cb(OTA_STATE_ERROR, msg, 0);
		});

	while (true) {
		ArduinoOTA.handle();

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
