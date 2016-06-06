/**
 * @file  CommonType.h  
 * @brief 常见类型定义
 * @author	   Vincent
 * @date	 2015-5-24 
 * @version  A001 
 * @copyright Vincent															  
 */
#ifndef COMMON_TYPE_H
#define COMMON_TYPE_H
/**
 * 4字节字符类型															   
 */
typedef unsigned int UINT;
/**
 * 1字节字符类型															   
 */
typedef unsigned char BYTE;
/** 
 * 2字节字符类型	
 *
 * 													 
 */
typedef unsigned short WORD;

/** 坐标系类型 */
typedef struct{
	int x;///<横坐标  
	int y;///<纵坐标
}Coordinator;
/// 枚举类型
enum COLOR{
	RED=0,	 ///  红色 
	GREEN=1, ///  绿色 
	YELLOW=2 /// 黄色 
};
/** 空的宏定义*/
#define NULL	0
#endif
