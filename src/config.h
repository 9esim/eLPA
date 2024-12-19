/***********************************************************************
 * @file config.h
 * @brief  CONFIG
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-06-08
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <Preferences.h>

class Config {
private:
    /* data */

public:
    String ssid;
    String password;

    Config();
    void setWifi(const char* ssid, const char* password);
    void getWifi(String& ssid, String& password);
    ~Config();
};

void config_init();

#endif // __CONFIG_H__