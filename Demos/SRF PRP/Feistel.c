/******************************************************************************
Modification Log
Version 		Date 			Author			Comments
1.0			5-29-2013		Rucha Shintre				Initial Version
1.1			6-05-2013		Pooja Desai,Rucha Shintre		Modified for use of a,b ; 
*******************************************************************************/

#include <tomcrypt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// block length is 32 bytes = 2^5
#define BLOCK_LENGTH 32

//1 gb = 2^30
#define GB pow(2,10)

//number of rounds
#define NOOFROUNDS 6

static int blocks;
static double index_bit_length;
static unsigned int * round1table;
static unsigned int * round2table;
static unsigned int * round3table;
static unsigned int * round4table;
static unsigned int * round5table;
static unsigned int * round6table;

void generateRoundFunctions(unsigned char * seed, unsigned int * bufint,int blocks); 

int main(int argc, char *argv[])
{
	//second argument is the file name to be permuted
	//atoi converted string to integer, we get the file size directly.
	if (argv[1])
    	blocks = atoi(argv[1]);
    else {
    	printf("passing block number as parameter\n");
    	exit(0);
    }
    	//printf("file size: %dG\n",filesize);
	
	//calculate blocklength 30 - 5 = 25 ( 2^30 stands for 1 gb, 2^5 for 32 bytes)
	//int temp = log2(GB) - log2(BLOCK_LENGTH);
	
    	//blocks = filesize * (1<<temp);
    	printf("block num: %d\n",blocks);
    	fflush(stdout);

	//find the bit length  of the each element of array to get number of blocks
	index_bit_length = log2(blocks);
	int bit = ceil(index_bit_length);
	    	printf("bit num: %d\n",bit);
	//declare for block indices table, and permuted indices table
	int * blockindices;
	int * prpblockindices;
	//allocate memory for input and output table
    	blockindices = (int *)malloc(blocks*sizeof(int));
    	prpblockindices = (int *)malloc(blocks*sizeof(int));

	//initialize block array to the block indices
    	int j=0;
	int i;
    	for (i=0;i<blocks;i++) {
		blockindices[i] = i;
    	}
	
	//hardcoding 6 seeds
	unsigned char * seed1 = "jiedai";
	unsigned char * seed2 = "ruchashintre";
	unsigned char * seed3 = "poojadesai";
	unsigned char * seed4 = "CMUMSE";
	unsigned char * seed5 = "BOSCH";
	unsigned char * seed6 = "jorge";

	//Generate 6 functions
	printf("Round 1\n");
	round1table = malloc(blocks*sizeof(unsigned int));
	generateRoundFunctions(seed1,round1table,blocks);
	printf("Round 2\n");
	round2table = malloc(blocks*sizeof(unsigned int));
	generateRoundFunctions(seed2,round2table,blocks);
	printf("Round 3\n");
	round3table = malloc(blocks*sizeof(unsigned int));
	generateRoundFunctions(seed3,round3table,blocks);
	printf("Round 4\n");
	round4table = malloc(blocks*sizeof(unsigned int));	
	generateRoundFunctions(seed4,round4table,blocks);
	printf("Round 5\n");	
	round5table = malloc(blocks*sizeof(unsigned int));
	generateRoundFunctions(seed5,round5table,blocks);
	printf("Round 6\n");
	round6table = malloc(blocks*sizeof(unsigned int));
	generateRoundFunctions(seed6,round6table,blocks);
	printf("6 tables generated");

	//using setting in the paper: change it later, to calculate a and b
	int a = 6;
	int b = 7;

	//get the keys for permutation
	
	//unsigned char * keyseed = "anappleadaykeepsadoctoraway";
	//int key = genkey(keyseed);
	
	//do this for six rounds
	for(i=0;i<blocks;i++){
		printf("Number of rounds: %d\n",i);
		prpblockindices[i]=Fe(NOOFROUNDS, a, b,i, blocks, bit);
	}
	
	for(i=0;i<blocks;i++){
		printf("%d -> %d\n", blockindices[i], prpblockindices[i]);		
	}
}

//generate 6 different functions and store in memory
void generateRoundFunctions(unsigned char * seed, unsigned int * bufint, int blocks)
{
	printf("before malloc\n");
	int err;
	int i=0;
	int j=0;

	unsigned char buf[4];
	
	//bufint = malloc(blocks*sizeof(unsigned int));

	
	int index = 0;
	prng_state prng; 
	
	if ((err = fortuna_start(&prng)) != CRYPT_OK) {
		printf("start error: %s\n", error_to_string(err));
	}
	if ((err = fortuna_add_entropy(seed, strlen(seed), &prng))!= CRYPT_OK) {
		printf("Add entropy error: %s\n", error_to_string(err));
	}
	if ((err = fortuna_ready(&prng)) != CRYPT_OK) {
		printf("Ready error: %s\n", error_to_string(err));
	}
		

	for(i=0;i<blocks;i++){
			
		fortuna_read(buf,sizeof(buf),&prng);
		bufint[i] = *(unsigned int *)buf;	
	}
	
	printf("%d blocks ", blocks);
	for(i=0;i<blocks;i++){

		printf("%u  ",bufint[i]);
	}
	fflush(stdout);

}

/*
//generate the seed for permuting 
int genkey(char *seed){

	int err;
	int i=0;
	int j=0;
	
	unsigned char buf[4];
	unsigned int bufint;
	
	int index = 0;
	prng_state prng; 
	
	if ((err = yarrow_start(&prng)) != CRYPT_OK) {
		printf("start error: %s\n", error_to_string(err));
	}
	if ((err = yarrow_add_entropy(seed, strlen(seed), &prng))!= CRYPT_OK) {
		printf("Add entropy error: %s\n", error_to_string(err));
	}
	if ((err = yarrow_ready(&prng)) != CRYPT_OK) {
		printf("Ready error: %s\n", error_to_string(err));
	}
		
	yarrow_read(buf,sizeof(buf),&prng);
	printf("prng info:%u",buf[0]);
	bufint = *(unsigned int *)buf;	
	
	return bufint;
}
*/

// buf = k = random number
int Fe(int r, int a, int b, int m, int blocks, int bit){
	
	int c = fe(r,a,b,m,bit);
	printf("c=%d,blocks=%d\n",c,blocks);
	if(c < blocks) {
		return c;
	}
	else {
		//perform cycle walking
		return Fe(r, a, b,c, blocks, bit);
	}
}

//computation of L , R
int fe(int r, int a, int b, long m,int bit){
	int tmp, j;
	int L = (int)m % a;
	int R = (int)m / a;
	for(j=1;j<=r;j++){
		int fval = applyRoundFunctions(j,R,bit);
		printf("fval=%d\n",fval);
		if(!(j%2)){
			tmp = (L ^ fval) % b;
		}else{
			tmp = (L ^ fval) % a;
		}
		L = R;
		R = tmp;	
	}
	
	if(!(r%2)){
		int kmn = (a*L+R);
		return (a*L+R);
	}else{
		int kmn = (a*R+L);
		return (a*R+L);
	}
}

//get the value of the function for given R
int applyRoundFunctions(int round, int R, int bit){
	printf("bit=%d\n",bit);
	switch(round){
		case 1: return round1table[R]&((1<<(bit))-1);
		case 2: return round2table[R]&((1<<(bit))-1);
		case 3: return round3table[R]&((1<<(bit))-1);
		case 4: return round4table[R]&((1<<(bit))-1);
		case 5: return round5table[R]&((1<<(bit))-1);
		case 6: return round6table[R]&((1<<(bit))-1);	
	}
}
	//print initial and final permutation
