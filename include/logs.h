/*********************************************************************
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date  : 2022-05-26 01:25:16.
 * Copyright (c) 2022. All rights reserved.
*********************************************************************/

#ifndef __LOGS_H__
#define __LOGS_H__

#include "types.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>

//#define _LOG_TO_FILE_
#ifdef _LOG_TO_FILE_
#define LOG_PATH "/home/root/linux_cpe"
#define LOG_DIR "logs"
#endif

#define ENABLE_DEBUG_LOGS

#define LOGS_BUFFER_LENGTH      2048
#define PRINT_BUFFER_LENGTH     LOGS_BUFFER_LENGTH

#ifdef LOG_COLOR_E
#undef LOG_COLOR_E
#endif // LOG_COLOR_E

#ifdef LOG_COLOR_W
#undef LOG_COLOR_W
#endif // LOG_COLOR_W

#ifdef LOG_COLOR_I
#undef LOG_COLOR_I
#endif // LOG_COLOR_I

#ifdef LOG_COLOR_D
#undef LOG_COLOR_D
#endif // LOG_COLOR_D

#ifdef LOG_COLOR_V
#undef LOG_COLOR_V
#endif // LOG_COLOR_V

#ifdef LOG_RESET_COLOR
#undef LOG_RESET_COLOR
#endif // LOG_RESET_COLOR

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_YELLOW  "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_EE       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_WW       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_II       LOG_COLOR(LOG_COLOR_YELLOW)
// #define LOG_COLOR_D
// #define LOG_COLOR_V

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

#define get_file_name(s) (strrchr(s, '/') ? (strrchr(s, '/') + 1) : (s))


#define DATE_TIME_STRING_LEN    40
#define TIME2STR_LEN            40

struct date_time_str {
    char string[DATE_TIME_STRING_LEN];
};

void get_datetime_string(struct date_time_str* t_str);
    
void logs_init(void);

int logs(const char* file, u32 line, const char* color, const char* fmt, ...);

int log_hex(const unsigned char* arr, size_t arr_len, const char* file, unsigned int line, const char* fmt, ...);

#define CONSTANT_STRING_DEFINE(s) const char *const s##_CONST_STRING = #s
#define CONSTANT_STRING_DECLARE(s) extern const char *const s##_CONST_STRING

#define LOGI(...)                   logs(__FILENAME__, __LINE__, LOG_COLOR_II , ##__VA_ARGS__)
#define LOGE(...)                   logs(__FILENAME__, __LINE__, LOG_COLOR_EE , ##__VA_ARGS__)
#define LOGW(...)                   logs(__FILENAME__, __LINE__, LOG_COLOR_WW , ##__VA_ARGS__)
// #define LOGD(...)                   logs(__FILENAME__, __LINE__, LOG_COLOR_D , ##__VA_ARGS__)
// #define LOGV(...)                   logs(__FILENAME__, __LINE__, LOG_COLOR_V , ##__VA_ARGS__)

#define LOG_HEX(_arr, _len, ...) log_hex(_arr, _len, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_MARK LOGI("")

// #ifdef DEBUG_LOG_ENABLE
// #define LOGI_TEST(logID, format, ...)   ECOMM_PRINTF(UNILOG_PLAT_AP, UNILOG_PLA_APP, logID, P_VALUE, format, ##__VA_ARGS__)
// #define ECLOGI(format, ...)             LOGI_TEST(__LINE__, format, ##__VA_ARGS__)
// #else
// #define ECLOGI(format, ...)             logs(__FILE__, __LINE__, format, ##__VA_ARGS__)
// #endif // DEBUG_LOG_ENABLE


#ifdef __cplusplus
}
#endif

#endif //__LOGS_H__
