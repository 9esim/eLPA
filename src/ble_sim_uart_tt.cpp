/***********************************************************************
 * @file ble_sim_uart_tt.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-31
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "ble_sim_uart_tt.h"
#include "smartcard.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <String.h>

#include "message_handler.h"
#include "logs.h"
#include "queue.h"
#include "lpac_wrapper.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID                "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

#define BLE_MTU                 256
typedef struct {
    uint8_t data[BLE_MTU];
    uint32_t rx_len;
} ble_pack_t;

#define BLE_DATA_QUEUE_LEN 10
#define BLE_DATA_QUEUE_ITEM_SIZE sizeof(ble_pack_t)

static QueueHandle_t ble_data_queue = NULL;

static int ble_recv_data(uint8_t* data, uint32_t len)
{
    uint16_t send_len = 0;
    ble_pack_t pack = { 0 };

    // LOGI("recv msg from ble[%d]: %s", len, (const char*)data);
    
    do {
        memset(&pack, 0, sizeof(ble_pack_t));
        pack.rx_len = len > BLE_MTU ? BLE_MTU : len;
        memcpy(pack.data, data + send_len, pack.rx_len);

        LOG_HEX(pack.data, pack.rx_len, "recv data[%d]: ", pack.rx_len);

        if (xQueueSend(ble_data_queue, &pack, 0) != pdPASS) {
            LOGE("Send data to ble_data_queue failed!");
            return -1;
        }

        send_len += pack.rx_len;
        len -= pack.rx_len;
    } while (len > 0);

    return 0;
}

static int ble_send_data(uint8_t* data, uint32_t datalen)
{
    uint16_t send_len = 0;
    uint16_t send_size;
    int len = datalen;
    
    // LOGI("send msg to ble[%d]: %s", len, (const char*)data);

    do {
        send_size = len > BLE_MTU ? BLE_MTU : len;
        pTxCharacteristic->setValue(data + send_len, send_size);
        pTxCharacteristic->notify();

        // LOG_HEX(data + send_len, send_size, "send data[%d]: ", send_size);
        // vTaskDelay(5 / portTICK_PERIOD_MS); // bluetooth stack will go into congestion, if too many packets are sent

        send_len += send_size;
        len -= send_size;
    } while (len > 0);

    LOGI("send data to ble done!");
    
    return 0;
}

int ble_sim_uart_tt_send_data(uint8_t* data, uint32_t len)
{
    return ble_send_data(data, len);
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer)
    {
        deviceConnected = false;
    }
    /***************** New - Security handled here ********************
    ****** Note: these are the same return values as defaults ********/
    // uint32_t onPassKeyRequest()
    // {
    //     LOGI("Server PassKeyRequest");
    //     return 123456;
    // }

    // bool onConfirmPIN(uint32_t pass_key)
    // {
    //     Serial.print("The passkey YES/NO number: ");LOGI(pass_key);
    //     return true;
    // }

    // void onAuthenticationComplete(ble_gap_conn_desc desc)
    // {
    //     LOGI("Starting BLE work!");
    // }
    /*******************************************************************/
};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0) {
            ble_recv_data((uint8_t*)rxValue.data(), rxValue.length());
        }
    }
};

void recv_data_task(void* pvParameters)
{
    ble_pack_t pack = { 0 };
    int ret = 0;
    uint8_t resp[UART_INTERFACE_BUFFER_SIZE];
    uint32_t rsp_len = 0;
    static uint8_t cmd[UART_INTERFACE_BUFFER_SIZE];
    static uint32_t cmd_len = 0;
    static uint8_t recv_state = 0;

    while (1) {
        if (xQueueReceive(ble_data_queue, &pack, portMAX_DELAY) == pdPASS) {
            LOGI("recv data[%d]: %s", pack.rx_len, (const char*)pack.data);
            message_handler((const char*)pack.data);
        }
    }

    LOGE("recv_data_task exit!");

    vTaskDelete(NULL);
}

#define BLE_SIM_UART_TASK_PRIORITY 5
#define BLE_SIM_UART_TASK_STACK_SIZE 8192 * 2

void ble_sim_uart_tt_init(void)
{
    // Create the BLE Device
    BLEDevice::init("eSIM_Writer");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        /******* Enum Type NIMBLE_PROPERTY now *******
            BLECharacteristic::PROPERTY_NOTIFY
            );
        **********************************************/
        // NIMBLE_PROPERTY::NOTIFY
        BLECharacteristic::PROPERTY_NOTIFY| BLECharacteristic::PROPERTY_READ
    );

    /***************************************************
     NOTE: DO NOT create a 2902 descriptor
     it will be created automatically if notifications
     or indications are enabled on a characteristic.

     pCharacteristic->addDescriptor(new BLE2902());
    ****************************************************/

    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        /******* Enum Type NIMBLE_PROPERTY now *******
                BLECharacteristic::PROPERTY_WRITE
                );
        *********************************************/
        // NIMBLE_PROPERTY::WRITE
        BLECharacteristic::PROPERTY_WRITE
        );

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();

    ble_data_queue = xQueueCreate(BLE_DATA_QUEUE_LEN, BLE_DATA_QUEUE_ITEM_SIZE);
    if (ble_data_queue == NULL) {
        LOGE("Create ble_data_queue failed!");
    }

    xTaskCreate(recv_data_task, "recv_data_task", BLE_SIM_UART_TASK_STACK_SIZE, NULL, BLE_SIM_UART_TASK_PRIORITY, NULL);

    LOGI("Waiting a client connection to notify...");
}

int ble_sim_uart_tt_task(void)
{

    // if (deviceConnected) {
    //     pTxCharacteristic->setValue(&txValue, 1);
    //     pTxCharacteristic->notify();
    //     txValue++;
    //     delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    // }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        LOGI("start advertising");
        oldDeviceConnected = deviceConnected;

        LOGI("Device %s disconnected!", BLEDevice::getAddress().toString().c_str());

        LOGI("Connected clinet(s): %d", pServer->getConnectedCount());
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;

        LOGI("Device %s connected!", BLEDevice::getAddress().toString().c_str());
        LOGI("Connected clinet(s): %d", pServer->getConnectedCount());
    }

    return 0;
}


void ble_sim_uart_tt_stop(void)
{
    if (ble_data_queue) {
        vQueueDelete(ble_data_queue);
        ble_data_queue = NULL;
    }

    // if (pServer) {
    //     pServer->getAdvertising()->stop();
    //     pServer->removeService(pServer->getServiceByUUID(SERVICE_UUID));
    //     pServer->stop();
    //     pServer = NULL;
    // }
}