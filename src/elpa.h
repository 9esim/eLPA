/***********************************************************************
 * @file eLPA.h
 * @brief  ELPA
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-06-08
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __ELPA_H__
#define __ELPA_H__

#include <Arduino.h>
#include <config.h>
#include <wifi_connection.h>

class eLPAClass
{
private:
    /* data */

public:
    String hardware;
    String firmware;
    String software;
    bool hasNewSoftware;
    String newSoftwareVersion;
    String mac;
    bool isCardInserted;

    Config* config;
    WiFiConnect* wifiConnect;

    String get_current_time();

    eLPAClass();
    ~eLPAClass();
};

extern eLPAClass *eLPA;

#endif // __ELPA_H__