/***********************************************************************
 * @file wifi_connection.h
 * @brief  WIFI_CONNECTION
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-04-11
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __WIFI_CONNECTION_H__
#define __WIFI_CONNECTION_H__

#include <Arduino.h>
#include <WiFi.h>

#ifdef __cplusplus
extern "C"{
#endif

class WiFiConnect
{
private:
    
public:
    String ssid;
    String password;

    WiFiClass& wifi = WiFi;
    bool isWiFiConnected;
    
    void startWiFi();
    void setDNS();
    int isConnected();
    WiFiConnect(String ssid, String password);
    ~WiFiConnect();
};

void sync_ntp_time(void);

#ifdef __cplusplus
}
#endif

#endif // __WIFI_CONNECTION_H__