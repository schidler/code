#include <stdio.h>


int isPrime(int num)
{
	if(num ==2 )
		return 1;
	int division=2;
	for(;division<num;division++)
	{
		if(num % division ==0 ) return 0;
	}
	return 1;

}


int main()
{


	return 0;
}
