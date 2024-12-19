/***********************************************************************
 * @file ble_sim_uart_tt.h
 * @brief  BLE_SIM_UART_TT
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-31
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __BLE_SIM_UART_TT_H__
#define __BLE_SIM_UART_TT_H__

#include "types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
    
void ble_sim_uart_tt_init(void);

int ble_sim_uart_tt_task(void);

int ble_sim_uart_tt_send_data(uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // __BLE_SIM_UART_TT_H__