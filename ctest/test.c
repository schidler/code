/**************************************************************
* Copyright (C) 2016-2016 All rights reserved.
* @Version: 1.0
* @Created: 2016-05-05 01:05
* @Author: menqi - 1083734876@qq.com
* @Description: 
*
* @History: 
**************************************************************/
#include <stdio.h>
#include <string.h>
// 0-1 交换问题

int swap_count_0_1(char* seq)
{
	int len=strlen(seq);
	int answer=0;
	int i,j;
	for(i=0,j=len-1;i<j;i++,j--)
	{
		for(;i<j && (seq[i]=='0');i++);
		//printf("test 1 i=%d,j=%d\n",i,j);
		for(;j>i && (seq[j]=='1');j--);
		//printf("test 2 i=%d,j=%d\n",i,j);
		if(i<j)++answer;
	}
	return answer;	
}



int main()
{
	char aaa[]="0000000001000000011";
	printf("%d\n",swap_count_0_1(aaa));
	return 0;
}
