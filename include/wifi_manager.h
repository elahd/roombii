
#pragma once

#include "config.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <Bluepad32.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#define MSG_NO_IP         "No IP Address"
#define MSG_NOT_CONNECTED "Not Connected"

enum WifiState {
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_CONNECTING,
	WIFI_STATE_GETTING_IP,
	WIFI_STATE_CONNECTED,
	WIFI_STATE_ERROR
};

enum OtaState {
	OTA_STATE_OFF,
	OTA_STATE_IDLE,
	OTA_STATE_ERROR,
	OTA_STATE_UPDATING,
	OTA_STATE_SUCCESS,
};

// typedef void (*f_connected_callback_t)();
typedef std::function<void(WifiState, String)>     WifiStateCallback;
typedef std::function<void(OtaState, String, int)> OtaStateCallback;

class WifiManager {
  public:
	bool        connected = false;
	const char *lanIp     = MSG_NO_IP;
	const char *ssid      = MSG_NOT_CONNECTED;
	WifiManager(const char *wifi_client_ssid, const char *wifi_client_pass, const char *device_name, const char *ota_pass, WifiStateCallback wifi_state_cb, OtaStateCallback ota_state_cb);
	WifiState wifi_state = WIFI_STATE_DISCONNECTED;
	OtaState  ota_state  = OTA_STATE_IDLE;
	void      begin();

  private:
	const char       *_ota_hostname;
	const char       *_ota_pass;
	const char       *_wifi_client_ssid;
	const char       *_wifi_client_pass;
	WifiStateCallback _wifi_state_cb;
	OtaStateCallback  _ota_state_cb;
	void              _wifiConnectedCallback(arduino_event_id_t event, arduino_event_info_t info);
	void              _wifiDisconnectedCallback(arduino_event_id_t event, arduino_event_info_t info);
	void              _wifiConnectingCallback(arduino_event_id_t event, arduino_event_info_t info);
	void              _wifiGotIpCallback(arduino_event_id_t event, arduino_event_info_t info);
	void              _wifiLostIpCallback(arduino_event_id_t event, arduino_event_info_t info);

	static void  _start_wifi_task_impl(void *_this);
	void         _wifi_task();
	TaskHandle_t _wifiTaskHandle;
};