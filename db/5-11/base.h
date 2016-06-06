/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-05-06 17:14
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#ifndef __BASE_H_
#define __BASE_H_
#include <stdio.h>
#include <stdlib.h>


// typedef unsigned char       u8; /**< UNSIGNED  8-bit data type */
// typedef unsigned short     u16; /**< UNSIGNED 16-bit data type */
// typedef unsigned int       u32; /**< UNSIGNED 32-bit data type */
// typedef unsigned long long u64; /**< UNSIGNED 64-bit data type */
// typedef signed char         s8; /**<   SIGNED  8-bit data type */
// typedef signed short       s16; /**<   SIGNED 16-bit data type */
// typedef signed int         s32; /**<   SIGNED 32-bit data type */
// typedef signed long long   s64; /**<   SIGNED 64-bit data type */


typedef char                Int8;
typedef signed short        Int16;
typedef int		    		Int32;
typedef long	    		Int64;
typedef unsigned char       Uint8;
typedef unsigned short	    Uint16;
typedef unsigned int        Uint32;
typedef unsigned long		Uint64;
typedef float		    	Float32;
typedef double				Float64;



#define LOG_ERROR(format, args...)  do { \
	fprintf(stdout, "ERROR %s, %d, "format, __FILE__, __LINE__, ##args);   \
	fflush(stdout);   \
} while (0)

#define LOG_PRINT(format, args...)  do { \
	fprintf(stdout, "Info: "format, ##args);   \
	fflush(stdout);   \
} while (0)

//todo
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define LOG_DEBUG(format, args...)  do { \
	fprintf(stdout, "Debug: "format, ##args);   \
	fflush(stdout);   \
} while (0)
#else
#define LOG_DEBUG(format, args...)
#endif

static inline unsigned  get_current_time(void){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	unsigned mseconds = now.tv_sec * 1000+ now.tv_nsec/1000000;
	return mseconds;
}

static void get_sys_time(char* strtime)  
{  

    time_t timep;  
    char regtime[20]={0};
    struct tm *p_tm;  
    timep = time(NULL);  
    p_tm = localtime(&timep);  
    //printf("size:%d\n",sizeof(strtime));
    strftime(regtime, sizeof(regtime), "%Y-%m-%d %H:%M:%S", p_tm);
    sprintf(strtime,"%s",regtime);
  
} 



static void start_time()
{
	time_t now = time(NULL);
	struct tm* p_tm = localtime(&now);
	LOG_PRINT("\t***************************************************************\n");
	LOG_PRINT("\t******huarui server start at the time: %d-%02d-%02d %02d:%02d:%02d******\n",
			(1900 + p_tm->tm_year), (1 + p_tm->tm_mon), p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
	LOG_PRINT("\t***************************************************************\n");
	LOG_DEBUG("huarui server  start: %u\n", get_current_time());
	return;
}


#endif
