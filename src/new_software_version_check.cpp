/***********************************************************************
 * @file new_software_version_check.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-07-23
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "new_software_version_check.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include "logs.h"
#include "cJSON.h"
#include "elpa.h"
#include "message_handler.h"


extern const uint8_t server_cert_pem_start[] asm("_binary_certs_ota_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_certs_ota_cert_pem_end");

void new_version_check_task(void* arg)
{
    const char* url = (const char*)arg;
    HTTPClient http;

    LOGI("Start to check...");

    LOGI("Free heap: %d", ESP.getFreeHeap());

    http.begin(url, (const char*)server_cert_pem_start);
    
    int httpCode = http.GET();

    if (httpCode > 0) {
        LOGI("HTTP code: %d", httpCode);

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            LOGI("Payload: %s", payload.c_str());
            cJSON* root = cJSON_Parse(payload.c_str());
            if (root == NULL) {
                LOGE("Failed to parse JSON");
                goto exit;
            }
            cJSON* version = cJSON_GetObjectItem(root, "version");
            if (version != NULL) {
                LOGI("New version: %s", version->valuestring);

                // parsing the version code
                int new_main_ver, new_mid_ver, new_last_ver;
                int cur_main_ver, cur_mid_ver, cur_last_ver;
                int num;
                num = sscanf(version->valuestring, "%d.%d.%d", &new_main_ver, &new_mid_ver, &new_last_ver);
                if (num != 3) {
                    LOGW("Parse new version code failed! params count: %d", num);
                    cJSON_Delete(root);
                    goto exit;
                }

                num = sscanf(eLPA->software.c_str(), "%d.%d.%d", &cur_main_ver, &cur_mid_ver, &cur_last_ver);
                if (num != 3) {
                    LOGW("Parse current version code failed! params count: %d", num);
                    cJSON_Delete(root);
                    goto exit;
                }

                if ((new_main_ver > cur_main_ver) || (new_mid_ver > cur_mid_ver) || (new_last_ver > cur_last_ver)) {
                    LOGI("A new software version available: %s --> %s", eLPA->software.c_str(), version->valuestring);
                    eLPA->hasNewSoftware = true;
                    eLPA->newSoftwareVersion = version->valuestring;

                    // Notify the new version to the BLE device
                    cJSON* elpa_info = cJSON_CreateObject();
                    cJSON_AddStringToObject(elpa_info, "cmd", "getElpaInfo");

                    char *elpa_info_str = cJSON_Print(elpa_info);

                    message_handler(elpa_info_str);

                    free(elpa_info_str);
                    
                    cJSON_Delete(elpa_info);
                }
            }

            cJSON_Delete(root);
        }

    }
    else {
        LOGE("HTTP failed, error: %s", http.errorToString(httpCode).c_str());
    }

exit:

    http.end();

    LOGI("Free heap: %d", ESP.getFreeHeap());

    // vTaskDelete(NULL);
}



int new_version_available_check(const char* url)
{
    LOGI("Checking for new version...");

    new_version_check_task((void*)url);
    // xTaskCreate(new_version_check_task, "new_version_check_task", 8192, (void *)url, 10, NULL);

    return 0;
}