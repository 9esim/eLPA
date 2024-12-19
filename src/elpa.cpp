/***********************************************************************
 * @file eLPA.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-06-08
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "elpa.h"
#include "logs.h"
#include <Arduino.h>
#include "https_ota.h"
#include "new_software_version_check.h"

#define NTP_SERVER1      "ntp1.aliyun.com"
#define NTP_SERVER2      "ntp2.aliyun.com"
#define NTP_SERVER3      "time.nist.gov"

bool is_ntp_synced = false;

void sync_ntp_time(void)
{
    if (is_ntp_synced) {
        return;
    }

    if (!eLPA->wifiConnect->isConnected()) {
        // LOGE("WiFi is not connected!");
        eLPA->wifiConnect->isWiFiConnected = false;
        is_ntp_synced = false;
        return;
    }

    if (!eLPA->wifiConnect->isWiFiConnected) {
        eLPA->wifiConnect->isWiFiConnected = true;
        eLPA->wifiConnect->setDNS();
        return;
    }

    configTzTime("CST-8", NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        LOGE("Failed to obtain time!");
        configTzTime("CST-8", NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
        return;
    }

    LOGI("Time obtained successfully!");


    is_ntp_synced = true;

    // auto check new version
    new_version_available_check(BASE_URL"version.json");
}

// Not sure if WiFiClientSecure checks the validity date of the certificate. 
// Setting clock just to be sure...
void setClock()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2) {
        delay(500);
        Serial.print(F("."));
        yield();
        nowSecs = time(nullptr);
    }

    Serial.println();
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);
    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}


String eLPAClass::get_current_time()
{
    // time format "2018-04-08 14:09:00"
    struct timeval time;
    time_t now;
    char buf[] = "2018-04-08 14:09:00";

    gettimeofday(&time, NULL);
    now = time.tv_sec;
    strftime(buf, 20, "%Y-%m-%d %T", localtime(&now));
    
    return String(buf);
}

eLPAClass::eLPAClass()
{
    char mac[20] = { 0 };
    snprintf(mac, sizeof(mac), "%llx", ESP.getEfuseMac());
    this->mac = String(mac);

    this->hardware = "2.0.0";
    this->firmware = "2.0.0";
    this->software = "2.2.1";
    this->hasNewSoftware = false;
    this->newSoftwareVersion = "";

    this->config = new Config();
    this->wifiConnect = new WiFiConnect(
        this->config->ssid.c_str(),
        this->config->password.c_str()
    );

    LOGI("Hardware: %s, Firmware: %s, Software: %s", this->hardware.c_str(), this->firmware.c_str(), this->software.c_str());

    LOGI("Device ID: %s", this->mac.c_str());
}

eLPAClass::~eLPAClass()
{
    delete this->config;
    delete this->wifiConnect;
}

eLPAClass *eLPA;