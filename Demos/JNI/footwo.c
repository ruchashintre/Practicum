#include <stdio.h>
#include <stdlib.h>

int main()
{
	int i;
	int ret = 0;
	for (i=0;i<1000000000;i++){
		ret = ret ^ 1;
  	}
	
  	printf("%d\n",ret);
}
