/***********************************************************************
 * @file lpac_wrapper.cpp
 * @brief 
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-05-28
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#include "lpac_wrapper.h"

#include "logs.h"
#include <stdio.h>


static int hex_to_str(const uint8_t* data, uint16_t data_len, char* str, uint16_t str_len)
{
    uint16_t i = 0;
    uint16_t j = 0;

    if (data_len * 2 + 1 > str_len) {
        return -1;
    }

    for (i = 0; i < data_len; i++) {
        sprintf(str + j, "%02X", data[i]);
        j += 2;
    }

    return j;
}

// 透传来自串口的数据
extern int main(int argc, char** argv);
int parse_command_into_cli(char* cmd)
{
    int argc = 0;
    char* argv[20] = { 0 };

    char* p = strtok(cmd, " ");
    while (p != NULL) {
        argv[argc++] = p;
        p = strtok(NULL, " ");
    }

    if (argc == 0) {
        return -1;
    }

    for (int i = 0; i < argc; i++) {
        LOGI("argv[%d]: %s", i, argv[i]);
    }

    return main(argc, argv);
}