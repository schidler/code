/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-26 22:04
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include "threadpool.h"
#include <stdio.h>
#include <string.h>
void* work(void* arg)
{
	char *p= (char*)arg;
	printf("threadpool callback fuction : %s.\n", p);
//	sleep(1);
}
#define WORK_NUM 10000
int main()
{
	struct threadpool *pool = threadpool_init(20, WORK_NUM);
	int i;
	
	for(i=0;i<WORK_NUM;i++)
	{
		char *buf=malloc(16);
		memset(buf,0,16);
		sprintf(buf,"%d",i);
		threadpool_add_job(pool, work, buf);
	}
	printf("success..\n");
	while(1);
	return 0;
}
