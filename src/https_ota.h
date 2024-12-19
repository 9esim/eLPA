/***********************************************************************
 * @file https_ota.h
 * @brief  HTTPS_OTA
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-07-21
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __HTTPS_OTA_H__
#define __HTTPS_OTA_H__

#include "new_software_version_check.h"

#ifdef __cplusplus
extern "C"{
#endif

int ota_start(const char* url);

int new_version_available_check(const char* url);

#ifdef __cplusplus
}
#endif

#endif // __HTTPS_OTA_H__