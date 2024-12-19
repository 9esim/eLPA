/***********************************************************************
 * @file new_software_version_check.h
 * @brief  NEW_SOFTWARE_VERSION_CHECK
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date: 2024-07-23
 * @Copyright (C) 2024 all right reserved
***********************************************************************/
#ifndef __NEW_SOFTWARE_VERSION_CHECK_H__
#define __NEW_SOFTWARE_VERSION_CHECK_H__

#ifdef __cplusplus
extern "C"{
#endif

#define BASE_URL "https://example.com/"

int new_version_available_check(const char* url);

#ifdef __cplusplus
}
#endif

#endif // __NEW_SOFTWARE_VERSION_CHECK_H__