/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-09 01:02
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
void swap(int *a,int *b)
{
	int temp;
	temp=*a;
	*a=*b;
	*b=temp;
}

/**
 * @brief bubble_sort 
 *
 * @Param: a[]
 * @Param: n
 */
void bubble_sort(int a[],int n)
{
	int i,j;
	int sign=0;
	for(i=0;i<n;i++)  //  n-1 times 
	{
		for(j=0;j<n-i-1;j++)
		{
			if(a[j]>a[j+1])
			{
				swap(a+j,a+j+1);
				sign=1;
			}
		}
		if(sign==0) break;
	}


}

/**
 * @brief insert_sort 
 *
 * @Param: a[]
 * @Param: ac
 *
 * Returns: 
 */
void insert_sort(int a[],int ac)
{	
	int max=a[0];
	int i;
	int j;
	for(i=1;i<ac;i++)
	{
		j=i-1; //  来了某个学生 位置为i ,前面学生位置为j  =i-1;  此时学生待插入
		while(1)
		{
			if(j<0)
				break;
			if(a[j]>a[j+1])
				swap(a+j,a+j+1);  //跟前面学生交换位置
			j--;


		}

	}


}

void shell_sort(int a[],int ac)
{


}

void merge_sort(int a[],int ac)
{
	int ac1,ac2,*ah1,*ah2;
	int *container;
	int i=0,j=0,k=0;
	if(ac<=1)return;
	/* split the array into two */
	ac1=ac/2;
	ac2=ac-ac1;
	ah1 = a + 0;
	ah2 = a + ac1;
	merge_sort(ah1, ac1);
	merge_sort(ah2, ac2);
	container = (int *) malloc(sizeof(int)*ac);

	while(i<ac1 && j<ac2) {
		if (ah1[i] <= ah2[j]) {
			container[k++] = ah1[i++];
		} 
		else {
			container[k++] = ah2[j++];
		}
	}
	while (i < ac1) {
		container[k++] = ah1[i++];
	}
	while (j < ac2) {
		container[k++] = ah2[j++];
	}

	/*copy back the sorted array*/
	for(i=0; i<ac; i++) {
		a[i] = container[i];
	}
	/*free space*/
	free(container);

}

void quick_sort(int a[],int ac)
{
	

	if(ac<=1)
		return;
	int left=0,right=ac-1;
	int i=left,j=right;  //哨兵  
	swap(a+0,a+ac/2);  //   将中间元素作为 挪到  a[0]  ,并作为基准 
	int temp=a[left];
	while(i!=j)  // 没有相遇
	{
		while(a[j]>=temp && i<j)
			j--;  //从右往左找  
		while(a[i]<= temp && i<j)
			i++;  //在从左往右找
		if(i<j)
			swap(a+i,a+j); //交换
	}
	
	//相遇时交换  
	swap(a+left,a+i);
	
	quick_sort(a,i);
	quick_sort(a+i+1,ac-i-1);

}



#define NUM 9
int main()
{
	int i;
	int a[]={1,9,3,4,6,5,16,8,14};
	//bubble_sort(a,NUM);
	//insert_sort(a,NUM);
	//merge_sort(a,NUM);
	quick_sort(a,NUM);
	for(i=0;i<NUM;i++)
		printf("%d ",a[i]);

	printf("\n");
}

