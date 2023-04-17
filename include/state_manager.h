#pragma once

#include "config.h"
#include "wifi_manager.h"

#include <Arduino.h>
#include <WS2812FX.h>

#ifndef PARTY_LENGTH_MS
#	define PARTY_LENGTH_MS 2000UL
#endif

#define QUEUE_LENGTH 10
#define ITEM_SIZE    sizeof(StateManager::StateMessage)

class StateManager {
  public:
	enum BluetoothState {
		BLUETOOTH_STATE_CONNECTED,
		BLUETOOTH_STATE_DISCONNECTED
	};

	enum StateComponent {
		WIFI,
		OTA,
		OTA_PROGRESS,
		BLUETOOTH,
		FRONT_BUMP,
		BIN_REMOVED,
		LIFTED,
		PARTY,
		WIIMOTE_CONNECTED,
		BATTERY,
	};

	struct States {
		WifiState      wifi          = WIFI_STATE_DISCONNECTED;
		BluetoothState bluetooth     = BLUETOOTH_STATE_DISCONNECTED;
		OtaState       ota           = OTA_STATE_OFF;
		int            ota_progress  = 0;
		bool           front_bumping = false;
		bool           bin_missing   = false;
		bool           lifted        = false;
		bool           partying      = false;
	};

	struct StateMessage {
		StateComponent msg_subject;
		union {
			WifiState      wifi_state;
			OtaState       ota_state;
			BluetoothState bluetooth_state;
			int            ota_progress;
			bool           front_bumping;
			bool           bin_missing;
			bool           lifted;
			bool           partying;
		} state;
	};

	StateManager(int right_lift_sensor_pin, int left_lift_sensor_pin, int bin_pin, int bump_pin);
	void              begin(void);
	SemaphoreHandle_t accept_user_drive_coordinates;

	QueueHandle_t xStatusQueueHandle;
	// uint8_t       ucStatusQueueStorageArea[QUEUE_LENGTH * ITEM_SIZE];

	WifiState      getWifiState(void);
	OtaState       getOtaState(void);
	int            getOtaProgress(void);
	BluetoothState getBluetoothState(void);
	bool           isPartying(void);

	void setWifiState(WifiState state);
	void setOtaState(OtaState state);
	void setOtaProgress(int progress);
	void setBluetoothState(BluetoothState state);
	void startParty(void);

  private:
	States _states;

	int _pin_right_lift_sensor;
	int _pin_left_lift_sensor;
	int _pin_bin;
	int _pin_bump;

	void _determineMode(void);
	void _sendStateMessage(StateComponent subject, void *state);

	static void  _start_state_task_impl(void *_this);
	void         _state_task();
	TaskHandle_t _stateTaskHandle;

	static void  _start_led_task_impl(void *_this);
	void         _led_task();
	TaskHandle_t _ledTaskHandle;

	WS2812FX _front_leds = WS2812FX(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
};