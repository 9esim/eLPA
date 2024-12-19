/*********************************************************************
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date  : 2022-05-26 01:26:42.
 * Copyright (c) 2022. All rights reserved.
*********************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>
#include "logs.h"

#ifdef ENABLE_DEBUG_LOGS
char buffer[LOGS_BUFFER_LENGTH] = { 0 };
char hex[PRINT_BUFFER_LENGTH] = { 0 };
#endif

void logs_init(void)
{
    Serial.begin(115200);
}

void get_datetime_string(struct date_time_str* t_str)
{
    struct timeval time;
    time_t now;
    char buf[20];// = "2018-04-08 14:09:00";

    gettimeofday(&time, NULL);
    now = time.tv_sec;
    strftime(buf, 20, "%Y-%m-%d %T", localtime(&now));
    snprintf(t_str->string, DATE_TIME_STRING_LEN, "%s.%06ld", buf, time.tv_usec);
}

int logs(const char* file, unsigned int line, const char* color, const char* fmt, ...)
{
#ifdef ENABLE_DEBUG_LOGS
    va_list list;
    struct date_time_str time;

    // if (output_logs == 0)
    //     return 0;

    get_datetime_string(&time);
    
    memset(buffer, 0, LOGS_BUFFER_LENGTH);

    va_start(list, fmt);
    vsnprintf(buffer, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

    Serial.printf("[%s]%s %-20s line:%-4d   %s" LOG_RESET_COLOR "\n", time.string, color, file, line, buffer);

#else
    Serial.printf("[%s]%s line:%-4d   9esim" LOG_RESET_COLOR "\n", time.string, color, line);
    
#endif
    return 0;
}

int log_hex(const unsigned char *arr, size_t arr_len, const char *file, unsigned int line, const char *fmt, ...)
{
#ifdef ENABLE_DEBUG_LOGS
    va_list list;
    size_t i = 0, j;
    struct date_time_str time;

    // if (output_logs == 0)
    //     return 0;

    get_datetime_string(&time);
    
    memset(buffer, 0, LOGS_BUFFER_LENGTH);
    memset(hex, 0, PRINT_BUFFER_LENGTH);

    va_start(list, fmt);
    vsnprintf(buffer, LOGS_BUFFER_LENGTH, fmt, list);
    va_end(list);

     memset(hex, 0, PRINT_BUFFER_LENGTH);
     for (j = 0; j < PRINT_BUFFER_LENGTH - 2;) {
         if (i >= arr_len)
             break;
         sprintf(&hex[j], "%02X", arr[i++]);
         j += 2;
     }
     hex[j] = 0;
      Serial.printf("[%s]" LOG_COLOR_II " %-20s line:%-4d   %s%s" LOG_RESET_COLOR "\n", time.string, file, line, buffer, hex);

#else
    Serial.printf("[%s]" LOG_COLOR_II " line:%-4d   %s" LOG_RESET_COLOR "\n", time.string, line, hex);
#endif
    return 0;
}