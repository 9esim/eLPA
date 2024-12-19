#include "esp32_http.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <euicc/interface.h>

#include <Arduino.h>
#include <HTTPClient.h>
#include "logs.h"

static const char* lpa_header[] = {
    "User-Agent: gsma-rsp-lpad",
    "X-Admin-Protocol: gsma/rsp/v2.2.0",
    "Content-Type: application/json",
    NULL,
};

static int http_interface_transmit(struct euicc_ctx* ctx, const char* url, uint32_t* rcode, uint8_t** rx, uint32_t* rx_len, const uint8_t* tx, uint32_t tx_len, const char** header)
{
    int fret = 0;
    int res;
    (*rx) = NULL;
    (*rcode) = 0;

    HTTPClient https;

    https.setTimeout(60000);
    https.begin(url);

    char key[128];
    char value[128];

    for (int i = 0; lpa_header[i] != NULL; i++) {
        // split header with ":"
        // e.g. "Content-Type: application/json"
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));

        if (sscanf(lpa_header[i], "%[^:]:%s", key, value) != 2) {
            LOGE("Invalid header: %s", lpa_header[i]);
            goto err;
        }

        https.addHeader(key, value);
    }

    https.setUserAgent("gsma-rsp-lpad");

    // traval headers
    for (int i = 0; i < https.headers(); i++) {
        LOGI("header[%d]: %s: %s", i, https.headerName(i).c_str(), https.header(i).c_str());
    }

    if (tx != NULL) {
        // https.addHeader("Content-Type", "application/json");
        res = https.POST((uint8_t*)tx, tx_len);
        LOGI("HTTPS POST %s, ret = %d", url, res);
    } else {
        res = https.GET();
        LOGI("HTTPS GET %s, ret = %d", url, res);
    }

    *rcode = res;
    
    if (res != HTTP_CODE_OK) {
        LOGE("Http request failed: %s\n", https.errorToString(res).c_str());
        goto err;
    }

    // print response body
    String payload = https.getString();
    *rx_len = payload.length();

    LOGI("Response[%d]: %s", *rx_len, payload.c_str());

    // *rx = reinterpret_cast<uint8_t*>(malloc(*rx_len));
    *rx = (uint8_t*)malloc(*rx_len);
    if (*rx == NULL) {
        LOGE("malloc failed of size %d", *rx_len);
        goto err;
    }

    memcpy(*rx, payload.c_str(), *rx_len);

    fret = 0;
    goto exit;

    err:
    fret = -1;

    exit:
    https.end();
    return fret;
}

static int libhttpinterface_init(struct euicc_http_interface* ifstruct)
{
    memset(ifstruct, 0, sizeof(struct euicc_http_interface));

    ifstruct->transmit = http_interface_transmit;

    return 0;
}

static int libhttpinterface_main(int argc, char** argv)
{
    return 0;
}

static void libhttpinterface_fini(void)
{}

const struct euicc_driver driver_http_esp32 = {
    .type = DRIVER_HTTP,
    .name = "esp32",
    .init = (int (*)(void*))libhttpinterface_init,
    .main = libhttpinterface_main,
    .fini = libhttpinterface_fini,
};
