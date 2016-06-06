 /** 
  * @file base.h
  * @brief 常见类型，调试开关定义 
  * @                                                               
  */
#ifndef __BASE_H_
#define __BASE_H_
#include <stdio.h>
#include <stdlib.h>

typedef char                Int8;     /**<   SIGNED  8-bit data type */
typedef signed short        Int16;    /**<   SIGNED 16-bit data type */
typedef int		    		Int32;    /**<   SIGNED 32-bit data type */
typedef long	    		Int64;    /**<   SIGNED 64-bit data type */
typedef unsigned char       Uint8;    /**< UNSIGNED  8-bit data type */
typedef unsigned short	    Uint16;   /**< UNSIGNED 16-bit data type */
typedef unsigned int        Uint32;   /**< UNSIGNED 32-bit data type */
typedef unsigned long		Uint64;   /**< UNSIGNED 64-bit data type */
typedef float		    	Float32;
typedef double				Float64;


/** 错误输出 */
#define LOG_ERROR(format, args...)  do { \
	fprintf(stdout, "ERROR %s, %d, "format, __FILE__, __LINE__, ##args);   \
	fflush(stdout);   \
} while (0)

/** 打印输出 */
#define LOG_PRINT(format, args...)  do { \
	fprintf(stdout, "Info: "format, ##args);   \
	fflush(stdout);   \
} while (0)

/**  调试开关 */
#define ENABLE_DEBUG


/** 调试输出 */
#ifdef ENABLE_DEBUG
#define LOG_DEBUG(format, args...)  do { \
	fprintf(stdout, "Debug: "format, ##args);   \
	fflush(stdout);   \
} while (0)
#else
#define LOG_DEBUG(format, args...)
#endif

/** 得到当前时间  以s为单位 */
static inline unsigned  get_current_time(void){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	unsigned mseconds = now.tv_sec * 1000+ now.tv_nsec/1000000;
	return mseconds;
}

/** 得到系统时间 */
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


/** 打印当前时间 */
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
