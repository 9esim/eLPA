/***********************************************************************
 * @file https_ota.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-07-21
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "https_ota.h"
#include "logs.h"

#include <Arduino.h>
#include <esp_https_ota.h>

#include <HttpsOTAUpdate.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "nvs.h"
#include "nvs_flash.h"

#include <sys/socket.h>
#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#include "cJSON.h"
#include "ble_sim_uart_tt.h"

#define HASH_LEN 32

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
/* The interface name value can refer to if_desc in esp_netif_defaults.h */
#if CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF_ETH
static const char* bind_interface_name = EXAMPLE_NETIF_DESC_ETH;
#elif CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF_STA
static const char* bind_interface_name = EXAMPLE_NETIF_DESC_STA;
#endif
#endif

extern const uint8_t server_cert_pem_start[] asm("_binary_certs_ota_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_certs_ota_cert_pem_end");

#define OTA_URL_SIZE 256

static u32 file_size = 0;
static u32 recv_size = 0, last_recv_size = 0;

static void report_upgrade_state(u32 total, u32 current, const char *state)
{
    char upgrade_info[256] = { 0 };

    const char* message_fmt = "{\"cmd\":\"upgrade\", \"state\":\"%s\", \"total\":%d, \"current\":%d}";

    snprintf(upgrade_info, sizeof(upgrade_info), message_fmt, state, file_size, recv_size);

    LOGI("send: %s", upgrade_info);

    ble_sim_uart_tt_send_data((uint8_t*)upgrade_info, strlen(upgrade_info));
} 

esp_err_t _http_event_handler(esp_http_client_event_t* evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
        LOGI("HTTP_EVENT_ERROR");
        break;
        case HTTP_EVENT_ON_CONNECTED: {
            LOGI("HTTP_EVENT_ON_CONNECTED");
            file_size = 0;
            recv_size = 0;
            last_recv_size = 0;
        } break;
        case HTTP_EVENT_HEADER_SENT:
        LOGI("HTTP_EVENT_HEADER_SENT");
        break;
        case HTTP_EVENT_ON_HEADER:
        LOGI("HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        if (strcmp(evt->header_key, "Content-Length") == 0) {
            file_size = atoi(evt->header_value);
            LOGI("File size: %d", file_size);
        }
        break;
        case HTTP_EVENT_ON_DATA: {
            // LOGI("HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            recv_size += evt->data_len;
            } break;
        case HTTP_EVENT_ON_FINISH: {
            LOGI("HTTP_EVENT_ON_FINISH");
            // Notify the upgrade process percent to the BLE device
        } break;
        case HTTP_EVENT_DISCONNECTED:
        LOGI("HTTP_EVENT_DISCONNECTED");
        break;
        // case HTTP_EVENT_REDIRECT:
        // LOGI("HTTP_EVENT_REDIRECT");
        // break;
    }
    return ESP_OK;
}

static char firmware_upgrade_url[OTA_URL_SIZE] = { 0 };

static void ota_timer_callback(TimerHandle_t xTimer)
{
    // report download speed
    LOGI("Speed: %d B/s", recv_size - last_recv_size);
    last_recv_size = recv_size;

    report_upgrade_state(file_size, recv_size, "downloading");
}

void simple_ota_example_task(void* pvParameter)
{
    char upgrade_info[256] = { 0 };

    LOGI("Starting OTA example task");

    esp_http_client_config_t config = {
        .url = firmware_upgrade_url,
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach,
#else
        .cert_pem = (char*)server_cert_pem_start,
#endif /* CONFIG_EXAMPLE_USE_CERT_BUNDLE */
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
        .if_name = &ifr,
#endif
    };

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    LOGI("Attempting to download update from %s", config.url);

    LOGI("Free heap: %d", ESP.getFreeHeap());

    // create a timer to report the download progress
    xTimerHandle timer = xTimerCreate("ota_timer", 1000 / portTICK_PERIOD_MS, pdTRUE, NULL, ota_timer_callback);
    xTimerStart(timer, 0);

    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        LOGI("OTA Succeed, Rebooting...");

        report_upgrade_state(file_size, recv_size, "succeed");

        esp_restart();
        while (1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    else {
        LOGE("Firmware upgrade failed!");
        report_upgrade_state(file_size, recv_size, "failed");
    }

    xTimerStop(timer, 0);
    xTimerDelete(timer, 0);

    LOGI("Free heap: %d", ESP.getFreeHeap());

    vTaskDelete(NULL);
}

static void print_sha256(const uint8_t* image_hash, const char* label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    LOGI("%s %s", label, hash_print);
}

static void get_sha256_of_partitions(void)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size = ESP_PARTITION_TABLE_OFFSET;
    partition.type = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

static TaskHandle_t ota_task_handler = NULL;

int ota_start(const char* url)
{
    LOGI("OTA example app_main start");
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    get_sha256_of_partitions();

    // clear the firmware_upgrade_url buffer
    memset(firmware_upgrade_url, 0, OTA_URL_SIZE);
    snprintf(firmware_upgrade_url, OTA_URL_SIZE, "%s", url);

    xTaskCreate(simple_ota_example_task, "ota_example_task", 8192, NULL, 10, &ota_task_handler);

    return 0;
}