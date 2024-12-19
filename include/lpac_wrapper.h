/***********************************************************************
 * @file lpac_wrapper.h
 * @brief  LPAC_WRAPPER
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-05-28
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __LPAC_WRAPPER_H__
#define __LPAC_WRAPPER_H__

#include "smartcard.h"

#ifdef __cplusplus
extern "C"{
#endif
    
#define UART_INTERFACE_BUFFER_SIZE      (UART_BUFFER_SIZE + 10)

int parse_command_into_cli(char* cmd);

#ifdef __cplusplus
}
#endif

#endif // __LPAC_WRAPPER_H__