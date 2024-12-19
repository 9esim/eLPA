/***********************************************************************
 * @file smartcard.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-29
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "smartcard.h"
#include "logs.h"

#include <esp_err.h>

#define VCC_POWER_SUPPLY_BY_GPIO

#ifdef VCC_POWER_SUPPLY_BY_GPIO
#define VCC_HIGH       HIGH
#define VCC_LOW        LOW
#else
#define VCC_HIGH       LOW
#define VCC_LOW        HIGH
#endif


uint32_t Smartcard::etu_to_ms(void)
{
    uint16_t temp;
    uint32_t etu = baudrate;

    temp = etu / (clk_freq / 1000);
    etu = etu % (clk_freq / 1000);

    if (etu) {
        temp += 1;
    }

    return temp;
}

void Smartcard::clk_init(void)
{
    uint32_t f = ledcSetup(this->clk_ledc_channel, this->clk_freq, 2);
    LOGI("Smartcard CLK freqency: %dHz", f);
    ledcAttachPin(this->clk_pin, this->clk_ledc_channel);
    ledcWrite(this->clk_ledc_channel, 0);
}

void Smartcard::clk_deinit(void)
{
    ledcDetachPin(this->clk_pin);
}

void Smartcard::clk_high(void)
{
    ledcWrite(this->clk_ledc_channel, 4);
}

void Smartcard::clk_low(void)
{
    ledcWrite(this->clk_ledc_channel, 0);
}

void Smartcard::clk_pulse(void)
{
    ledcWrite(this->clk_ledc_channel, 2);
}

void Smartcard::sim_gpio_init(void)
{
    pinMode(this->rst_pin, OUTPUT);
    pinMode(this->vcc_pin, OUTPUT);

    digitalWrite(this->rst_pin, LOW);
    digitalWrite(this->vcc_pin, VCC_LOW);
}

void Smartcard::sim_gpio_deinit(void)
{
    digitalWrite(this->rst_pin, LOW);
    digitalWrite(this->vcc_pin, VCC_LOW);
}

void Smartcard::rst_high(void)
{
    digitalWrite(this->rst_pin, HIGH);
}

void Smartcard::rst_low(void)
{
    digitalWrite(this->rst_pin, LOW);
}

void Smartcard::vcc_high(void)
{
    digitalWrite(this->vcc_pin, VCC_HIGH);
}

void Smartcard::vcc_low(void)
{
    digitalWrite(this->vcc_pin, VCC_LOW);
}

uart_t* Smartcard::sim_uart_init(uint8_t uart_num, uint32_t baudrate, gpio_num_t uart_rx, gpio_num_t uart_tx)
{
    uart_t* uart = NULL;

    // UART_DATA_8_BITS | UART_PARITY_EVEN | UART_STOP_BITS_2 | UART_HW_FLOWCTRL_DISABLE;
    // this->uart_config = UART_DATA_8_BITS << 2;
    // uart_config |= UART_PARITY_EVEN;
    // uart_config |= UART_STOP_BITS_2 << 4;
    this->uart_config = SERIAL_8E2;
    // configure the UART port
    uart = uartBegin(uart_num, baudrate, this->uart_config, uart_rx, uart_tx, this->uart_buffer_size, this->uart_buffer_size, 0, 122);
    if (uart == NULL) {
        LOGE("Failed to initialize UART port %d", uart_num);
        return NULL;
    }

    this->baudrate = uartGetBaudRate(uart);

    LOGI("Smartcard runs on UART %d, initialized with baudrate %d", uart_num, this->baudrate);

    // uartSetMode(uart, MODE_RS485_HALF_DUPLEX);
    uartSetMode(uart, UART_MODE_RS485_HALF_DUPLEX);
    uartSetRxTimeout(uart, READ_TOUT);
    uartFlush(uart);

    return uart;
}

void Smartcard::sim_uart_deinit(uint8_t uart_num)
{
    uartEnd(this->uart_num);
    this->uart = NULL;
}

void Smartcard::set_sim_uart_mode_to_receive(bool is_receive)
{
    bool res;
    if (!is_receive) {
        res = uartSetPins(this->uart_num, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    } else {
        res = uartSetPins(this->uart_num, UART_RX, UART_TX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    }
}

int Smartcard::uart_send_data(const uint8_t* data, size_t len)
{
    int ret = 0;
    set_sim_uart_mode_to_receive(false);
    ret = uart_write_bytes(this->uart_num, data, len);
    ESP_ERROR_CHECK(uart_wait_tx_done(this->uart_num, 100 / portTICK_RATE_MS));
    set_sim_uart_mode_to_receive(true);

    return ret;
}

void Smartcard::set_baudrate(uint32_t baudrate)
{
    this->baudrate = baudrate;
    uartSetBaudRate(this->uart, baudrate);
    this->baudrate = uartGetBaudRate(this->uart);
    LOGI("Smartcard baudrate set to %d", this->baudrate);
}

void Smartcard::cold_reset(void)
{
    memset(&ATR, 0, sizeof(ATR_t));

    sim_gpio_init();
    clk_init();
    vcc_high();
    clk_pulse();
    delay(200 / CLK_FREQ);
    uart_flush_input(this->uart_num);
    delay(40000 / CLK_FREQ);

    rst_high();

    int ret = get_atr();
    if (ret < 0) {
        LOGE("Failed to get ATR!");
    }
    else {
        LOG_HEX(&ATR.TS, ATR_len, "ATR[%d]: ", ATR_len);
        ppss_exchange();
    }
}

void Smartcard::warm_reset(void)
{
    // FIXME: implement warm reset
}

int Smartcard::get_atr(void)
{
    uint8_t atr[64], * p = atr;

    LOGI("Waiting for ATR...");

    int atr_len = try_read(atr, sizeof(atr));
    if (atr_len <= 0) {
        LOGE("Failed to get ATR!");
        return -1;
    }

    while (*p == 0 && atr_len > 0) {
        p++;
        atr_len--;
    }

    if (atr_len <= 0) {
        LOGE("Invalid ATR!");
        return -1;
    }

    memset(&ATR, 0, sizeof(ATR_t));
    memcpy(&ATR, p, atr_len);

    this->ATR_len = atr_len;

    LOG_HEX(atr, atr_len, "ATR[%d]: ", atr_len);

    return atr_len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

Smartcard::Smartcard(uint8_t uart_num, uint32_t clk_freq, uint32_t clk_ledc_channel,
                     gpio_num_t rst_pin, gpio_num_t vcc_pin, gpio_num_t clk_pin,
                     gpio_num_t uart_rx, gpio_num_t uart_tx)
{
    this->uart_num = uart_num;
    this->rst_pin = rst_pin;
    this->vcc_pin = vcc_pin;
    this->clk_pin = clk_pin;
    this->uart_rx = uart_rx;
    this->uart_tx = uart_tx;
    this->uart_buffer_size = UART_BUFFER_SIZE;
    this->clk_freq = clk_freq;
    this->baudrate = clk_freq / 372;
    this->isCardInserted = false;

    this->clk_ledc_channel = CLK_BIND_LEDC_CHANNEL;

    this->uart = sim_uart_init(this->uart_num, this->baudrate, this->uart_rx, this->uart_tx);
    if (this->uart == NULL) {
        LOGE("Failed to initialize UART port %d", this->uart_num);
        return;
    }

    LOGI("SIM card cold reset");
    cold_reset();

    LOGI("SIM card start to work!");
}

Smartcard::~Smartcard()
{
    sim_uart_deinit(this->uart_num);
    sim_gpio_deinit();
    clk_deinit();
}

int Smartcard::try_read(uint8_t* data, uint32_t len, uint32_t timeout)
{
    return uart_read_bytes(this->uart_num, data, len, timeout / portTICK_RATE_MS);
}

int Smartcard::try_write(const uint8_t* data, uint32_t len, uint32_t timeout)
{
    return uart_send_data(data, len);
}

int Smartcard::read(uint8_t* data, uint32_t len)
{
    return uartReadBytes(this->uart, data, len, 100 / portTICK_RATE_MS);
}

int Smartcard::write(const uint8_t* data, uint32_t len)
{
    return uart_send_data(data, len);
}

int Smartcard::send(const uint8_t* data, uint32_t len)
{
    return uart_send_data(data, len);
}

int Smartcard::recv(uint8_t* data, uint32_t len)
{
    return uartReadBytes(this->uart, data, len, 100 / portTICK_RATE_MS);
}

/************************************************************************************
 * TPDU Exchange
 * @param apdu: APDU command
 * @param apdu_len: APDU command length
 * @param resp: response data
 * @param resp_len: response data length
 * @return: 0 success, 1 timeout, 2 APDU format error, 3 communication error
*************************************************************************************/
int Smartcard::tpdu_exchange(const uint8_t* apdu, uint32_t apdu_len, uint8_t* resp, uint32_t* resp_len)
{
    uint8_t err, wait, recvFlag;
    uint16_t i, lc, le;
    uint8_t pc;
    uint8_t INS = apdu[1];
    uint16_t overTim;

    uint8_t tpdu[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

    uart_flush_input(this->uart_num);

    overTim = 5000;

    memcpy(tpdu, apdu, 5);

    if (apdu_len == 4) {
        tpdu[4] = 0x00;
        lc = le = 0;
    } else if (apdu_len == 5) {
        lc = 0;
        le = apdu[4] == 0 ? 256 : apdu[4];
    } else {
        if (apdu_len == apdu[4] + 5) {
            lc = apdu[4];
            le = 0;
        } else if (apdu_len == apdu[4] + 6) {
            lc = apdu[4];
            le = apdu[apdu_len - 1];
        } else {
            LOGE("Invalid APDU case!");
            return -1;
        }
    }

    // send 5 bytes of APDU header
    int len = try_write(tpdu, 5);
    LOG_HEX(tpdu, len, "TPDU-C[%d]: ", len);

    wait = 1;
    apdu_len = 0;
    *resp_len = 0;
    recvFlag = 0;
    while (wait) {
        wait = 0;
        err = try_read(&pc, 1, 6000);
        if (err <= 0) {
            LOGE("Failed to receive data! err = %d", err);
            return -1;
        } else {
            LOG_HEX(&pc, 1, "TPDU-R[%d]: ", 1);

            if ((pc >= 0x90 && pc <= 0x9F) || (pc >= 0x60 && pc <= 0x6F)) {
                switch (pc) {
                    case 0x60:
                    wait = 1;
                    break;
                    default:
                    resp[*resp_len] = pc;
                    (*resp_len)++;

                    err = try_read(&pc, 1, overTim);
                    if (err <= 0) {
                        LOGE("Failed to receive data! err = %d", err);
                        return -1;
                    }

                    resp[*resp_len] = pc;
                    (*resp_len)++;
                    break;
                }
            }
            else {
                resp[*resp_len] = pc;
                pc ^= INS;
                if (pc == 0) {
                    if (recvFlag == 0 && lc > apdu_len) {
                        try_write(apdu + 5 + apdu_len, lc - apdu_len);
                        apdu_len = lc;
                        recvFlag = 1;
                    }

                    if ((recvFlag == 1 || lc == 0) && le > *resp_len) {
                        for (i = 0; i < le - *resp_len; i++) {
                            err = try_read(resp + *resp_len + i, 1, overTim);
                            if ((err <= 0) && i < le - *resp_len) {
                                *resp_len = i;
                                LOGE("Failed to receive data! err = %d", err);
                                return -1;
                            }
                        }
                        *resp_len = le;
                    }
                    wait = 1;
                } else if (pc == 0xFF) {
                    if (recvFlag == 0 && lc > apdu_len) {
                        try_write(&apdu[5 + apdu_len], 1);
                        apdu_len++;
                        if (apdu_len == lc) {
                            recvFlag = 1;
                        }
                    }

                    if ((recvFlag == 1 || lc == 0) && le > *resp_len) {
                        err = try_read(resp + *resp_len, 1, overTim);
                        if (err <= 0) {
                            LOGE("Failed to receive data! err = %d", err);
                            break;
                        }
                        (*resp_len)++;
                    }
                    wait = 1;
                }
                else {
                    if (INS == 0xC0) {
                        (*resp_len)++;
                        if ((recvFlag == 1 || lc == 0) && le > *resp_len) {
                            for (i = 0; i < le - *resp_len; i++) {
                                err = try_read(resp + *resp_len + i, 1, overTim);
                                if ((err <= 0) && i < le - *resp_len) {
                                    *resp_len = i;
                                    LOGE("Failed to receive data! err = %d", err);
                                    return -1;
                                }
                            }
                            *resp_len = le;
                        }
                        wait = 1;
                    }
                    else {
                        LOGE("Invalid APDU!");
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}

int Smartcard::apdu_exchange(const uint8_t* apdu, uint32_t apdu_len, uint8_t* resp, uint32_t* resp_len)
{
    int ret;

    if (apdu_len < 4) {
        LOGE("Invalid APDU length!");
        return -1;
    }

    ret = tpdu_exchange(apdu, apdu_len, resp, resp_len);

    if (ret < 0) {
        LOGE("Failed to exchange APDU!");
        resp[0] = 0x69;
        resp[1] = 0x00;
        *resp_len = 2;
        return -1;
    }

    return *resp_len;
}

static const u32 F_Table[16] = {
    372, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768, 1024, 1536, 2048, 0, 0
};

static const u32 D_Table[16] = { 0, 1, 2, 4, 8, 16, 32, 0, 12, 20, 0, 0, 0, 0, 0, 0 };

void Smartcard::ppss_exchange(void)
{
    uint8_t ppss[] = {0xFF, 0x10, 0x00, 0x00};
    uint8_t resp[4];
    uint32_t resp_len = sizeof(resp);
    u32 Fi, Di;

    Fi = F_Table[((ATR.T[0] >> 4) & (u8)0x0F)];
    Di = D_Table[(ATR.T[0] & (u8)0x0F)];

    if (Fi == 0 || Di == 0) {
        LOGE("Invalid Fi or Di, Fi: %d, Di: %d, Use default.", Fi, Di);
        return;
    }

    ppss[2] = ATR.T[0];
    ppss[3] = 0xFF ^ 0x10 ^ ATR.T[0];
    LOG_HEX(ppss, sizeof(ppss), "PPSS-C[%d]: ", sizeof(ppss));
    send(ppss, sizeof(ppss));

    resp_len = try_read(resp, sizeof(resp), 100);
    LOG_HEX(resp, resp_len, "PPSS-R[%d]: ", resp_len);

    if (resp_len >= 4 && memcmp(resp, ppss, 4) == 0) {
        LOGI("PPSS exchange success!");

        baudrate = clk_freq * Di / Fi;
        set_baudrate(baudrate);
        this->isCardInserted = true;
    }
    else {
        LOGE("PPSS exchange failed!");
    }
}


Smartcard* smartcard = NULL;
bool card_inserted = false;

void hotplug_init(void)
{
    pinMode(SIM_DC, INPUT_PULLUP);
}
void card_detect(void)
{
    if (digitalRead(SIM_DC) == LOW) {
        card_inserted = false;
    }
    else {
        card_inserted = true;
    }
}