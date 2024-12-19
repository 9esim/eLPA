/***********************************************************************
 * @file uart_tt.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-31
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "uart_tt.h"

#include <Arduino.h>
#include "logs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "lpac_wrapper.h"

static HardwareSerial listen_serial = Serial;

static TaskHandle_t uart_task_handler = NULL;

static void uart_task(void* arg)
{
    uint8_t data[UART_INTERFACE_BUFFER_SIZE] = { 0 };
    uint8_t resp[UART_INTERFACE_BUFFER_SIZE] = { 0 };
    uint32_t rsp_len = 0;
    uint32_t available_len = 0;

    int ret;

    while (1) {
        available_len = listen_serial.available();
        if (available_len == 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        available_len = listen_serial.readBytes(data, available_len);
        if (available_len > 0) {
            // LOG_HEX(data, available_len, "UART-Rx[%d]: ", available_len);
            data[available_len] = '\0';
            LOGI("UART-Rx[%d]: %s", available_len, (char*)data);

            parse_command_into_cli((char *)data);
            // if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x01) {
            //     LOGI("Reset SIM card!");
            //     delete smartcard;
            //     smartcard = new Smartcard();
            // } else {
                // if (smartcard == NULL) {
                //     smartcard = new Smartcard();
                // }
            //     ret = smartcard->apdu_exchange(data, available_len, resp, &rsp_len);
            // }

            // if (ret <= 0) {
            //     LOGE("Failed to exchange apdu!");
            // } else {
            //     LOG_HEX(resp, rsp_len, "UART-Tx[%d]: ", rsp_len);
            //     // LOGI("UART-Tx[%d]: %s", rsp_len, (char *)resp);

            //     listen_serial.write(resp, rsp_len);
            // }
        }
    }
}

void uart_tt_start(void)
{
    if (listen_serial != Serial) {
        listen_serial.setRxBufferSize(UART_INTERFACE_BUFFER_SIZE);
        listen_serial.setTxBufferSize(UART_INTERFACE_BUFFER_SIZE);
        listen_serial.begin(115200, SERIAL_8N1, GPIO_NUM_14, GPIO_NUM_13);
    }
    xTaskCreate(uart_task, "uart_task", 8192, NULL, 10, &uart_task_handler);
}

void uart_tt_stop(void)
{
    if (uart_task_handler) {
        vTaskDelete(uart_task_handler);
        uart_task_handler = NULL;
    }
}
