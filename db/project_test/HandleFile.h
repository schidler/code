/**le HandleFile.h 
 * @brief 操作文件
 * @details 所有涉及文件操作
 * @author       Vincent
 * @date     2015-5-24 
 * @version  A001 
 * @par Copyright (c):  Vincent 
 */ 
#ifndef HANDLE_FILE_H
#define HANDLE_FILE_H
#include "CommonType.h"


UINT ReadFile(UINT fileNameLen, BYTE *fileName, UINT dataLen, BYTE *fileData);


UINT WriteFile(UINT fileNameLen, BYTE *fileName, BYTE *fileData);


UINT EraseFile(UINT fileNameLen, BYTE *fileName);
#endif
