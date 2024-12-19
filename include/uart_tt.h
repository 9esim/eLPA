/***********************************************************************
 * @file uart_tt.h
 * @brief  UART_TT: transparent transmission by UART
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-03-31
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __UART_TT_H__
#define __UART_TT_H__

#ifdef __cplusplus
extern "C"{
#endif

void uart_tt_start(void);

void uart_tt_stop(void);

#ifdef __cplusplus
}
#endif

#endif // __UART_TT_H__