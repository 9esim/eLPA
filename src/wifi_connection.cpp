/***********************************************************************
 * @file wifi_connection.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-04-11
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "wifi_connection.h"
#include "logs.h"

WiFiConnect::WiFiConnect(String ssid, String password)
{
    this->ssid = ssid;
    this->password = password;
    this->isWiFiConnected = false;
}

void WiFiConnect::startWiFi()
{
    LOGI("Connecting to %s ...", ssid.c_str());

    wifi.begin(ssid.c_str(), password.c_str());
}

void WiFiConnect::setDNS()
{
    LOGI("IP address: %s", wifi.localIP().toString().c_str());
    LOGI("Subnet mask: %s", wifi.subnetMask().toString().c_str());
    LOGI("Gateway IP: %s", wifi.gatewayIP().toString().c_str());

    // mac address
    LOGI("MAC address: %s", WiFi.macAddress().c_str());

    wifi.config(wifi.localIP(), wifi.gatewayIP(), wifi.subnetMask(),
        IPAddress(1, 1, 1, 1), IPAddress(8, 8, 8, 8));

    LOGI("DNS1 IP: %s", wifi.dnsIP(0).toString().c_str());
    LOGI("DNS2 IP: %s", wifi.dnsIP(1).toString().c_str());
}

int WiFiConnect::isConnected()
{
    return wifi.isConnected();
}

WiFiConnect::~WiFiConnect()
{

}