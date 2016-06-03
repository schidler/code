/**************************************************************
* Copyright (C) 2016-2016 All rights reserved.
* @Version: 1.0
* @Created: 2016-04-11 21:58
* @Author: menqi - 1083734876@qq.com
* @Description: 
*  求从小大的第1500 个丑数      
* @History: 
**************************************************************/

#include <stdio.h>


int IsUgly(int num)
{
	while(num%2==0)
		num /=2;
	while(num%3==0)
		num /=3;
	while(num%5==0)
		num /=5;
	return (num==1)? 1: 0;
}
/*

   method 1
int main()
{
	int count=0;
	int num=0;
	while(count<1500){
		++num;
		if(IsUgly(num))
			count++;
		//printf("count=%d\n",count);
	}
	printf("the num is %d \n",num);
	return 0;
}*/
int Min(int a,int b,int c)
{
	a=a<b?a:b;
	if(c<a)return c;
	return a;
}

int main()
{
	int count=1;
	int arr[1500]={0};
	arr[0]=1;
	int index2=0,index3=0,index5=0;
	while(count<1500)
	{
		int min=Min(arr[index2]*2,arr[index3]*3,arr[index5]*5);
		arr[count]=min;	
		while(arr[index2]*2<=arr[count])index2++;
		while(arr[index3]*3<=arr[count])index3++;
		while(arr[index5]*5<=arr[count])index5++;
		count++;
	}
	printf("the num is %d \n",arr[count-1]);
}
