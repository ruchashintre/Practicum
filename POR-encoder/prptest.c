#include "FeistelPRP.h"
#include "keygenwrapper.h"

int main() {

	int a = 32;
	int * prptable = malloc(a*sizeof(int));
	printf("here \n");
	prptable = prp(a, "aaa");
	printf("here \n");
	int i;
	for (i=0;i<a;i++) {
		printf("%d -> %d \n",i,prptable[i]);
	}
}
