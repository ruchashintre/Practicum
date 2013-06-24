#include <tomcrypt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "keygenwrapper.h"
//#include "jg_timing.h"

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
int* prp(int blocks, unsigned char* key);
