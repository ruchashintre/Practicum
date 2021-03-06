#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//contains bit strings
int beta[50];

int main(int argc, char *argv[]) {
	int i;
	int prp[27];
	for (i=0;i<50;i++) {
		//srand(time(NULL));
		beta[i] = rand();
	}

	for (i=0;i<27;i++) {
		prp[i] = permute(i,0,27,0);
		printf("%d -> %d\n",i,prp[i]);
	}
	printf("======= permutation table ======\n");
	for (i=0;i<27;i++) {
		printf("%d -> %d\n",i,prp[i]);
	}		
}

int permute(int x, int a, int l, int d) {
	printf("permute x=%d, a=%d, l=%d, d=%d\n",x,a,l,d);
	if (l==1)
		return a;
	int t = countZero(beta[d],a,l);
	if ((beta[d] & (1<<(a+x)))==0) {
		int newx = countZero(beta[d],a,x);
		return permute(newx, a, t, d+1);
	}
	else {
		int newx = countOne(beta[d],a,x);
		return permute(newx, a+t, countOne(beta[d],a,l),d+1);
	}
}

int countZero(int beta, int a, int x) {
	return x-countOne(beta,a,x);
}
		
int countOne(int beta, int a, int x) {
	int counter=0,i;
	for (i=a; i<a+x; i++) {
		if ((beta & (1<<i))!=0) {
			counter = counter+1;
		}
	}
	return counter;
}
