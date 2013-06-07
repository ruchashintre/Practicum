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

int blocks;
int index_bit_length;
unsigned int * round1table;
unsigned int * round2table;
unsigned int * round3table;
unsigned int * round4table;
unsigned int * round5table;
unsigned int * round6table;

void generateRoundFunctions(unsigned char * seed, unsigned int * bufint,int blocks); 

int main(int argc, char *argv[])
{
	//second argument is the file name to be permuted
	//atoi converted string to integer, we get the file size directly.
    	int blocks = atoi(argv[1]);
    	//printf("file size: %dG\n",filesize);
	
	//calculate blocklength 30 - 5 = 25 ( 2^30 stands for 1 gb, 2^5 for 32 bytes)
	//int temp = log2(GB) - log2(BLOCK_LENGTH);
	
    	//blocks = filesize * (1<<temp);
    	printf("block num: %d\n",blocks);

	//find the bit length  of the each element of array to get number of blocks
	index_bit_length = log2(blocks);
	
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
	generateRoundFunctions(seed1,round1table,blocks);
	printf("Round 2\n");
	generateRoundFunctions(seed2,round2table,blocks);
	printf("Round 3\n");
	generateRoundFunctions(seed3,round3table,blocks);
	printf("Round 4\n");	
	generateRoundFunctions(seed4,round4table,blocks);
	printf("Round 5\n");	
	generateRoundFunctions(seed5,round5table,blocks);
	printf("Round 6\n");
	generateRoundFunctions(seed6,round6table,blocks);
	printf("6 tables generated");

	//using setting in the paper: change it later, to calculate a and b
	int a = 185360;
	int b = 185368;

	//get the keys for permutation
	
	//unsigned char * keyseed = "anappleadaykeepsadoctoraway";
	//int key = genkey(keyseed);
	
	//do this for six rounds
	for(i=0;i<blocks;i++){
		printf("Number of rounds");
		prpblockindices[i]=Fe(NOOFROUNDS, a, b,i);
	}
	
	for(i=0;i<blocks;i++){
		printf("%d -> %d", blockindices[i], prpblockindices[i]);		
	}
}

//generate 6 different functions and store in memory
void generateRoundFunctions(unsigned char * seed, unsigned int * bufint, int blocks)
{
	printf("before malloc");
	int err;
	int i=0;
	int j=0;

	unsigned char buf[4];
	
	bufint = malloc(blocks*sizeof(unsigned int));
	
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
	

//printf("%d blocks ", blocks);
	//for(i=0;i<blocks;i++){
		
	//	printf("%u  ",bufint[i]);
	//}

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
int Fe(int r, int a, int b, int m){
	
	printf("I am  in Main Feistel function");

	int c = fe(r,a,b,m);
	if(c < blocks)
		return c;
	else
		//perform cycle walking
		return Fe(r, a, b,c);
}

//computation of L , R
int fe(int r, int a, int b, long m){
	printf("I am  in Main Feistel function");
	int tmp, j;
	int L = (int)m % a;
	int R = (int)m / a;
	for(j=1;j<=r;j++){
		if(!(j%2)){
			tmp = (L ^ applyRoundFunctions(j,R)) % b;
		}else{
			tmp = applyRoundFunctions(j,R) % a;
		}
		L = R;
		R = tmp;	
	}
	
	if(!(r%2)){
		return (a*L+R);
	}else{
		return (a*R+L);
	}
}

//get the value of the function for given R
int applyRoundFunctions(int round, int R){
	
	switch(round){
		case 1: return round1table[R];
		case 2: return round2table[R];
		case 3: return round3table[R];
		case 4: return round4table[R];
		case 5: return round5table[R];
		case 6: return round6table[R];	
	}
}
	//print initial and final permutation
