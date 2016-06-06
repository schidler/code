#include "DoxygenFile.h"
#include "HandleFile.h"

/** 
 * 写入一些内容
 * @param[in]   fileNameLen    文件名长度
 * @param[in]    fileName    文件名
 * @param[out]    fileData    文件数据
 * @return        0，执行成功，非0，失败，详见
 * @ref            RetCode.h 
 */ 
int HandleData(UINT fileNameLen,BYTE *fileName, BYTE *fileData)
{
	UINT retCode;
	retCode = ReadFile(0,NULL,0,NULL);
	return retCode;
}
