/***********************************************************************
 * @file iso7816.c
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-05-02
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "iso7816.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <euicc/hexutil.h>
#include <euicc/interface.h>
#include <smartcard.h>
#include <malloc.h>
#include <logs.h>

#define EUICC_INTERFACE_BUFSZ 264

// #define APDU_ST33_MAGIC "\x90\xBD\x36\xBB\x00"
#define APDU_TERMINAL_CAPABILITIES "\x80\xAA\x00\x00\x0A\xA9\x08\x81\x00\x82\x01\x01\x83\x01\x07"
#define APDU_OPENLOGICCHANNEL "\x00\x70\x00\x00\x01"
#define APDU_CLOSELOGICCHANNEL "\x00\x70\x80\xFF\x00"
#define APDU_SELECT_HEADER "\x00\xA4\x04\x00\xFF"

static int lowlevel_transmit(uint8_t* rx, uint32_t* rx_len, const uint8_t* tx, const uint8_t tx_len)
{
    int ret;
    uint32_t n = 0;

    LOG_HEX(tx, tx_len, "TX[%2d]", tx_len);

    if (!smartcard) {
        LOGI("smartcard is not initialized");
        smartcard = new Smartcard();
        if (!smartcard->isCardInserted) {
            LOGE("No card inserted");
            
            return -1;
        }
    }

    ret = smartcard->apdu_exchange(tx, tx_len, rx, rx_len);
    if (ret < 0) {
        LOGE("apdu exchange failed");
        return -1;
    }

    LOG_HEX(rx, *rx_len, "RX[%2d]", *rx_len);

    return 0;
}

static void iso7816_logic_channel_close(uint8_t channel)
{
    uint8_t tx[sizeof(APDU_CLOSELOGICCHANNEL) - 1];
    uint8_t rx[EUICC_INTERFACE_BUFSZ];
    uint32_t rx_len;

    memcpy(tx, APDU_CLOSELOGICCHANNEL, sizeof(tx));
    tx[3] = channel;

    rx_len = sizeof(rx);

    lowlevel_transmit(rx, &rx_len, tx, sizeof(tx));
}

static int iso7816_logic_channel_open(const uint8_t* aid, uint8_t aid_len)
{
    int channel = 0;
    uint8_t tx[EUICC_INTERFACE_BUFSZ];
    uint8_t* tx_wptr;
    uint8_t rx[EUICC_INTERFACE_BUFSZ];
    uint32_t rx_len;

    if (aid_len > 32) {
        goto err;
    }

    rx_len = sizeof(rx);
    if (lowlevel_transmit(rx, &rx_len, (const uint8_t*)APDU_OPENLOGICCHANNEL, sizeof(APDU_OPENLOGICCHANNEL) - 1) < 0) {
        goto err;
    }

    if (rx_len != 3) {
        goto err;
    }

    if ((rx[1] & 0xF0) != 0x90) {
        goto err;
    }

    channel = rx[0];

    tx_wptr = tx;
    memcpy(tx_wptr, APDU_SELECT_HEADER, sizeof(APDU_SELECT_HEADER) - 1);
    tx_wptr += sizeof(APDU_SELECT_HEADER) - 1;
    memcpy(tx_wptr, aid, aid_len);
    tx_wptr += aid_len;

    tx[0] = (tx[0] & 0xF0) | channel;
    tx[4] = aid_len;

    rx_len = sizeof(rx);
    if (lowlevel_transmit(rx, &rx_len, tx, tx_wptr - tx) < 0) {
        goto err;
    }

    if (rx_len < 2) {
        goto err;
    }

    switch (rx[rx_len - 2]) {
        case 0x90:
        case 0x61:
            return channel;
        default:
            goto err;
    }

    err:
    if (channel) {
        iso7816_logic_channel_close(channel);
    }

    return -1;
}

static int apdu_interface_connect(struct euicc_ctx* ctx)
{
    uint8_t rx[EUICC_INTERFACE_BUFSZ];
    uint32_t rx_len;

    if (!smartcard) {
        LOGI("smartcard is not initialized");
        smartcard = new Smartcard();
        if (!smartcard->isCardInserted) {
            LOGE("No card inserted");

            return -1;
        }
    }

    LOGI("Opened smartcard success!");

    rx_len = sizeof(rx);
    lowlevel_transmit(rx, &rx_len, (const uint8_t*)APDU_TERMINAL_CAPABILITIES, sizeof(APDU_TERMINAL_CAPABILITIES) - 1);

    return 0;
}

static void apdu_interface_disconnect(struct euicc_ctx* ctx)
{
    if (smartcard) {
        delete smartcard;
        smartcard = NULL;
    }
}

// static uint8_t euicc_rsp_buffer[EUICC_INTERFACE_BUFSZ];

static int apdu_interface_transmit(struct euicc_ctx* ctx, uint8_t** rx, uint32_t* rx_len, const uint8_t* tx, uint32_t tx_len)
{
    *rx = (uint8_t*)malloc(EUICC_INTERFACE_BUFSZ);
    if (!*rx) {
        LOGE("SCardTransmit() RX buffer alloc failed");
        return -1;
    }
    // *rx = euicc_rsp_buffer;
    *rx_len = EUICC_INTERFACE_BUFSZ;

    if (lowlevel_transmit(*rx, rx_len, tx, tx_len) < 0) {
        free(*rx);
        *rx_len = 0;
        return -1;
    }

    return 0;
}

static int apdu_interface_logic_channel_open(struct euicc_ctx* ctx, const uint8_t* aid, uint8_t aid_len)
{
    return iso7816_logic_channel_open(aid, aid_len);
}

static void apdu_interface_logic_channel_close(struct euicc_ctx* ctx, uint8_t channel)
{
    iso7816_logic_channel_close(channel);
}

static int libapduinterface_init(struct euicc_apdu_interface* ifstruct)
{
    memset(ifstruct, 0, sizeof(struct euicc_apdu_interface));

    ifstruct->connect = apdu_interface_connect;
    ifstruct->disconnect = apdu_interface_disconnect;
    ifstruct->logic_channel_open = apdu_interface_logic_channel_open;
    ifstruct->logic_channel_close = apdu_interface_logic_channel_close;
    ifstruct->transmit = apdu_interface_transmit;

    return 0;
}

static int libapduinterface_main(int argc, char** argv)
{
    LOGI("driver iso7816 in use!");
    return 0;
}

static void libapduinterface_fini(void)
{
    LOGI("driver iso7816 fini");
}

const struct euicc_driver driver_apdu_iso7816 = {
    .type = DRIVER_APDU,
    .name = "iso7816",
    .init = (int (*)(void*))libapduinterface_init,
    .main = libapduinterface_main,
    .fini = libapduinterface_fini,
};
