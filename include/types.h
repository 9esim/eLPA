/*********************************************************************
 * Author: xiaoxiao
 * E-mail: dev@9esim.com
 * Date  : 2022-06-10 10:08:21.
 * Copyright (c) 2022 all rights reserved.
*********************************************************************/

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C"{
#endif

typedef char 			    s8;
typedef unsigned char 	    u8;
typedef short 			    s16;
typedef unsigned short 	    u16;
typedef int 			    s32;
typedef unsigned int 	    u32;

#define size_array(a) sizeof(a)/sizeof(a[0])

#ifdef __cplusplus
}
#endif
#endif
