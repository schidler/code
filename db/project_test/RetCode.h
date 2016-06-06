/** 
 * @file RetCode.h
 * @brief 错误码返回值 
 * @author       Vincent
 * @date     2015-5-24 
 * @version  A001 
 * @par Copyright (c):  Vincent                                                                 
 */

#ifndef RET_CODE_H
#define RET_CODE_H


/**@name    执行状态
 * @{
 */
#define SUCCESS            0x00000000        ///<执行成功
#define ERR_PARA_LEN    0x00000001        ///<长度错误
#define ERR_NULL_POINT    0x00000002        ///<空指针错误
#define ERR_PARA_TYPE    0x00000003        ///<参数类型错误
/** @}*/

#endif
