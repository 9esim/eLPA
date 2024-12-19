/***********************************************************************
 * @file message_handler.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-06-08
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "message_handler.h"
#include <cJSON.h>
#include "logs.h"
#include "elpa.h"
#include "lpac_wrapper.h"
#include "ble_sim_uart_tt.h"
#include "new_software_version_check.h"
#include "https_ota.h"

static int set_wifi(const cJSON* cmd)
{
    cJSON* ssid = cJSON_GetObjectItem(cmd, "ssid");
    cJSON* password = cJSON_GetObjectItem(cmd, "password");
    if (ssid == NULL || password == NULL) {
        LOGE("get ssid or password failed");
        return -1;
    }

    eLPA->config->setWifi(ssid->valuestring, password->valuestring);
    eLPA->wifiConnect->wifi.disconnect();
    
    delete eLPA->wifiConnect;

    eLPA->wifiConnect = new WiFiConnect(eLPA->config->ssid, eLPA->config->password);

    eLPA->wifiConnect->startWiFi();

    eLPA->wifiConnect->wifi.reconnect();

    return 0;
}

static int get_wifi(const cJSON* cmd)
{
    cJSON* wifi = cJSON_CreateObject();
    cJSON_AddStringToObject(wifi, "cmd", "getWifiInfo");

    cJSON* data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "ssid", eLPA->config->ssid.c_str());
    cJSON_AddStringToObject(data, "password", eLPA->config->password.c_str());
    cJSON_AddBoolToObject(data, "connected", eLPA->wifiConnect->isConnected());
    cJSON_AddNumberToObject(data, "rssi", eLPA->wifiConnect->wifi.RSSI());
    cJSON_AddStringToObject(data, "time", eLPA->get_current_time().c_str());

    cJSON_AddItemToObject(wifi, "data", data);

    char* wifi_str = cJSON_Print(wifi);

    LOGI("wifi info: %s", wifi_str);

    ble_sim_uart_tt_send_data((uint8_t*)wifi_str, strlen(wifi_str));

    cJSON_Delete(wifi);

    free(wifi_str);

    return 0;
}


extern int output_logs;
static int get_elpa_info(const cJSON* cmd)
{
    cJSON* elpa_info = cJSON_CreateObject();
    cJSON_AddStringToObject(elpa_info, "cmd", "getElpaInfo");

    cJSON* data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "ssid", eLPA->config->ssid.c_str());
    cJSON_AddStringToObject(data, "password", eLPA->config->password.c_str());
    cJSON_AddBoolToObject(data, "connected", eLPA->wifiConnect->isConnected());
    cJSON_AddNumberToObject(data, "rssi", eLPA->wifiConnect->wifi.RSSI());
    cJSON_AddStringToObject(data, "time", eLPA->get_current_time().c_str());
    cJSON_AddStringToObject(data, "hardware", eLPA->hardware.c_str());
    cJSON_AddStringToObject(data, "firmware", eLPA->firmware.c_str());
    cJSON_AddStringToObject(data, "software", eLPA->software.c_str());
    cJSON_AddBoolToObject(data, "hasNewSoftware", eLPA->hasNewSoftware);
    cJSON_AddStringToObject(data, "newSoftwareVersion", eLPA->newSoftwareVersion.c_str());
    cJSON_AddStringToObject(data, "mac", eLPA->mac.c_str());

    cJSON_AddItemToObject(elpa_info, "data", data);

    char* elpa_info_str = cJSON_Print(elpa_info);

    LOGI("elpa info: %s", elpa_info_str);

    ble_sim_uart_tt_send_data((uint8_t*)elpa_info_str, strlen(elpa_info_str));

    cJSON_Delete(elpa_info);

    free(elpa_info_str);

    return 0;
}

static int check_update(const cJSON* cmd)
{
    cJSON* url = cJSON_GetObjectItem(cmd, "url");
    if (url == NULL) {
        LOGI("use default url");
        new_version_available_check(BASE_URL"version.json");
    }
    else {
        LOGI("Update software from %s", url->valuestring);
        new_version_available_check(url->valuestring);
    }

    return 0;
}

static int update_software(const cJSON* cmd)
{
    cJSON* url = cJSON_GetObjectItem(cmd, "url");
    if (url == NULL || url->valuestring == NULL || strlen(url->valuestring) == 0) {
        LOGI("use default url");
        ota_start(BASE_URL"firmware.bin");
    }
    else {
        LOGI("Update software from %s", url->valuestring);

        ota_start(url->valuestring);
    }
    
    return 0;
}


typedef struct {
    const char* cmd;
    int (*handler)(const cJSON* cmd);
} command_handler_t;

static command_handler_t command_handlers[] = {
    {"setWifiInfo", set_wifi},
    {"getWifiInfo", get_wifi},
    {"getElpaInfo", get_elpa_info},
    {"upgradeSoftware", update_software},
    {"checkUpdate", check_update},
};

int message_handler(const char* message)
{
    cJSON* root = cJSON_Parse(message);

    LOGI("message: %s", message);

    if (root == NULL) {
        LOGW("Is not a json message, try to parse it as a command");

        if (!card_inserted) {
            cJSON_Delete(root);
            cJSON* resp = cJSON_CreateObject();
            cJSON_AddStringToObject(resp, "cmd", "cardStatus");
            cJSON_AddBoolToObject(resp, "cardInserted", false);
            char* resp_str = cJSON_Print(resp);
            ble_sim_uart_tt_send_data((uint8_t*)resp_str, strlen(resp_str));

            LOGI("card not inserted, send card status to ble: %s", resp_str);

            cJSON_Delete(resp);

            free(resp_str);

            return -1;
        }

        return parse_command_into_cli((char*)message);
    }

    cJSON* cmd = cJSON_GetObjectItem(root, "cmd");
    if (cmd == NULL) {
        LOGE("get cmd failed");
        cJSON_Delete(root);
        return -1;
    }

    for (size_t i = 0; i < sizeof(command_handlers) / sizeof(command_handler_t); i++) {
        if (strcmp(cmd->valuestring, command_handlers[i].cmd) == 0) {
            LOGI("handle command: %s", cmd->valuestring);
            command_handlers[i].handler(root);
            break;
        }
    }
    
    cJSON_Delete(root);

    return 0;
}