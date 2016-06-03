/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-28 16:23
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include <iostream>
#include <map>
#include <stdio.h>
using namespace std;

typedef map<int,int>  SockMap;
SockMap appmap;
SockMap hostmap;
typedef SockMap::iterator SockItor;
int main()
{

	appmap[35]=3;
	hostmap[40]=3;
	SockItor itor=hostmap.find(40);
	int fd=itor->second;
	SockItor it; 
	int num;
	for(it=appmap.begin();it!=appmap.end(); it++) 
	{ 
		if(it->second==fd)
			num=it->first;


	}
		printf("hellp map..fd:%d\n",fd);
		printf("hellp map..num:%d\n",num);
		return 0;
}
