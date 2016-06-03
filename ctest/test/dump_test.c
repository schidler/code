/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-05-04 23:43
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/


/*core_dump_test.c*/
#include <stdio.h>
char *str = "test";
void core_test(){
	str[1] = 'T';
}

int main(){
	core_test();
	return 0;
}
