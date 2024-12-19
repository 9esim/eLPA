/***********************************************************************
 * @file smartcard.h
 * @brief  SMARTCARD
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-29
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __SMARTCARD_H__
#define __SMARTCARD_H__

#include <Arduino.h>

#include <driver/gpio.h>
#include <driver/uart.h>

#define SIM_RST                     GPIO_NUM_9
#define SIM_VCC                     GPIO_NUM_45
#define SIM_CLK                     GPIO_NUM_12
#define SIM_DC                      GPIO_NUM_3
#define CLK_FREQ                    3580000
#define CLK_BIND_LEDC_CHANNEL       0
#define SIM_UART_NUM                UART_NUM_2
#define UART_RX                     GPIO_NUM_11
#define UART_TX                     GPIO_NUM_17 // not used
#define READ_TOUT                   (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks
#define CLK_FREQ                    3580000
#define CLK_BIND_LEDC_CHANNEL       0
#define UART_BUFFER_SIZE            256

// The Answer-to-Reset (ATR) structure is defined in ISO/IEC 7816-3:2006(E) 8.2
// ATR = [TS, T0, T1, ..., Tk, (TA1), (TB1), (TC1), (TD1), (TA2), (TB2), (TC2), (TD2), ...]
typedef struct {
    uint8_t TS; // Initial character
    uint8_t T0; // Format character
    uint8_t T[32]; // Historical characters
    uint8_t TCK; // Check character
} ATR_t;

class Smartcard {
private:
    gpio_num_t rst_pin;
    gpio_num_t vcc_pin;
    gpio_num_t clk_pin;
    gpio_num_t uart_rx;
    gpio_num_t uart_tx;
    uint32_t baudrate;
    uint32_t uart_config;
    uint32_t clk_freq;
    uint32_t clk_ledc_channel;

    // utils
    uint32_t etu_to_ms(void);
    
    // SIM card clock control
    void clk_init(void);
    void clk_deinit(void);
    void clk_high(void);
    void clk_low(void);
    void clk_pulse(void);

    // SIM card GPIO control
    void sim_gpio_init(void);
    void sim_gpio_deinit(void);
    void rst_high(void);
    void rst_low(void);
    void vcc_high(void);
    void vcc_low(void);

    // SIM card UART control
    uart_t* sim_uart_init(uint8_t uart_num, uint32_t baudrate, gpio_num_t uart_rx, gpio_num_t uart_tx);
    void sim_uart_deinit(uint8_t uart_num);
    void set_sim_uart_mode_to_receive(bool is_receive = true);
    int uart_send_data(const uint8_t* data, size_t len);
    void set_baudrate(uint32_t baudrate);

    void cold_reset(void);
    void warm_reset(void);
    int get_atr(void);
    void ppss_exchange(void);

public:
    uint8_t uart_num;
    uart_t* uart;
    uint32_t uart_buffer_size;
    // uint8_t ATR[64];
    ATR_t ATR;
    uint8_t ATR_len;
    bool isCardInserted;

    Smartcard(uint8_t uart_num = SIM_UART_NUM, uint32_t clk_freq = CLK_FREQ, uint32_t clk_ledc_channel = CLK_BIND_LEDC_CHANNEL,
              gpio_num_t rst_pin = SIM_RST, gpio_num_t vcc_pin = SIM_VCC, gpio_num_t clk_pin = SIM_CLK,
              gpio_num_t uart_rx = UART_RX, gpio_num_t uart_tx = UART_TX);
    ~Smartcard();

    int try_read(uint8_t* data, uint32_t len, uint32_t timeout = 100);
    int try_write(const uint8_t* data, uint32_t len, uint32_t timeout = 100);

    int read(uint8_t* data, uint32_t len);
    int write(const uint8_t* data, uint32_t len);

    int send(const uint8_t* data, uint32_t len);
    int recv(uint8_t* data, uint32_t len);

    int apdu_exchange(const uint8_t* apdu, uint32_t apdu_len, uint8_t* resp, uint32_t* resp_len);
    int tpdu_exchange(const uint8_t* tpdu, uint32_t tpdu_len, uint8_t* resp, uint32_t* resp_len);
};

void hotplug_init(void);

void card_detect(void);

extern Smartcard* smartcard;
extern bool card_inserted;

#endif // __SMARTCARD_H__