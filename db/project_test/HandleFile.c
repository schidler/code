#include "HandleFile.h"
#include "RetCode.h"
#include "AddrDefine.h"

/** 
 * 读取文件
 * @param[in]    fileNameLen    文件名长度
 * @param[in]   fileName    文件名
 * @param[in]    dataLen        数据长度
 * @param[out]  fileData    输出数据
 * @return        0，执行成功，非0，失败，详见
 * @ref            RetCode.h
 * @see
 * @note
 */ 
UINT ReadFile(UINT fileNameLen, BYTE *fileName, UINT dataLen, BYTE *fileData)
{
	return SUCCESS;
}

/** 
 * 写入文件
 * @param[in]    fileNameLen    文件名长度
 * @param[in]   fileName    文件名
 * @param[out]  fileData    输出数据
 * @return        0，执行成功，非0，失败，详见
 * @ref            RetCode.h
 * @see
 * @note 
 */ 
UINT WriteFile(UINT fileNameLen, BYTE *fileName, BYTE *fileData)
{
	return SUCCESS;
}

/** 
 * 擦除文件
 * @param[in]    fileNameLen    文件名长度
 * @param[in]   fileName    文件名
 * @return        0，执行成功，非0，失败，详见
 * @ref            RetCode.h
 * @see
 * @note 
 */ 
UINT EraseFile(UINT fileNameLen, BYTE *fileName)
{
	return SUCCESS;
}
