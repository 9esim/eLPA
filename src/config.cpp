/***********************************************************************
 * @file config.c
 * @brief
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-06-08
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "config.h"
#include "logs.h"

#define SSID1 			"9eSIM-2.4G"
#define PASSWORD1 		"link_up_more"

#define PREF_NAMESPACE      "config"
#define PREF_SSID           SSID1
#define PREF_PASSWORD       PASSWORD1

static Preferences preferences;

void config_init()
{
    preferences.begin(PREF_NAMESPACE, false);

    if (!preferences.isKey("ssid") || !preferences.isKey("password")) {
        LOGW("Preferences not found, setting defaults");
        preferences.putString("ssid", PREF_SSID);
        preferences.putString("password", PREF_PASSWORD);
    }
}

void Config::setWifi(const char* ssid, const char* password)
{
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    this->ssid = String(ssid);
    this->password = String(password);
}

void Config::getWifi(String& ssid, String& password)
{
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    this->ssid = ssid;
    this->password = password;
}

Config::Config()
{
    this->ssid = preferences.getString("ssid", "");
    this->password = preferences.getString("password", "");

    LOGI("SSID: %s, Password: %s", this->ssid.c_str(), this->password.c_str());
}

Config::~Config()
{
}