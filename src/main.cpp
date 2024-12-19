#include <Arduino.h>
#include "logs.h"
#include "uart_tt.h"
#include "ble_sim_uart_tt.h"
#include "wifi_connection.h"
#include "smartcard.h"
#include <WiFi.h>
#include <elpa.h>
#include <config.h>

// #ifndef LED_BUILTIN
#define LED_BUILTIN 21
// #endif

#define USB_DP_PIN 19
#define USB_DM_PIN 20

#define LED_PIN    21
void setup()
{
	pinMode(USB_DP_PIN, INPUT);
	pinMode(USB_DM_PIN, INPUT);

	logs_init();
	config_init();
	eLPA = new eLPAClass();
	if (eLPA == NULL) {
		LOGE("Failed to create eLPA object!");
		return;
	}

	eLPA->wifiConnect->startWiFi();
	
	// uart_tt_start();
	ble_sim_uart_tt_init();
	LOGI("Waiting for WiFi connection...");
	// WiFiConnect wifi(config.ssid.c_str(), config.password.c_str());
	pinMode(LED_BUILTIN, OUTPUT);
	// pinMode(GPIO_NUM_38, OUTPUT);
	// digitalWrite(GPIO_NUM_38, HIGH);
	// pinMode(LED_PIN, OUTPUT);
	// digitalWrite(LED_PIN, HIGH);

	hotplug_init();
}

void loop() {
	// put your main code here, to run repeatedly:
	sync_ntp_time();
	// request("https://jsonplaceholder.typicode.com/posts?userId=1");
	// sleep(10);

	card_detect();

	if (ble_sim_uart_tt_task()) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(10);
		digitalWrite(LED_BUILTIN, LOW);
		delay(10);
	} else {
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
	}
}
