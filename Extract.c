#include <stdio.h>
#include <stdlib.h>
#include "eccwrapper.h"
#include "FeistelPRP.h"
#include "encwrapper.h"
#include "math.h"

#define alpha 10
#define delta 0.25
#define BLOCK_SIZE 32
#define n 255
#define k 233
#define d 32
#define w 4096
#define v 1024

//Challenge structure
typedef struct {
	int j;
	int k_j_c;
	int u;
}chal;

//decoding file blocks structure
typedef struct {
	char ** file_blocks;
	int * frequency;
} decoding;

void inner_decoding(decoding *d1, char * codeword, int * v_chal_indices);

int extract(int t,unsigned char * masterkey, int * prptable, char * filename){
	
	decoding * d1;
	int i,j,u,size,index;
	char * codeword,mac;
	chal * c;
	unsigned char * k_chal, k_j_c, k_ecc_perm, k_ecc_enc, k_mac;
	char ** F;
	char str[999];
	FILE *fp1,*fp2;
	char * r_file="recovered";
	int * v_chal_indices;

	//use master key and call keygen to generate all the keys here.	
	//use new function written by Jie. 
	keygen(masterkey);
	
	//open encoded file for reading	
  	if ((fp1 = fopen(filename, "r")) == NULL){
	         printf("couldn't open input file for reading.\n");
	         return -1;
    	}
	
	//open output file for writing
	if ((fp2 = fopen(r_file, "w")) == NULL){
	         printf("couldn't open output file for writing.\n");
	         return -1;
    	}

	//allocate memory for d1
	
	d1 = (decoding *) malloc (sizeof(decoding)*alpha*t);
	if(d1 == NULL) {
		fprintf(stderr, "failed to allocate memory for d.\n");
		return -1;
	}
	
		
	size = alpha*(t/v);

	//allocate memory for the challenge set
	if ((c = (chal *)malloc(sizeof(chal)*size))== NULL) {
		fprintf(stderr, "failed to allocate memory for challenges.\n");
		return -1;
	}
	
	// populate challenge set	
	for (j=0;j<size;j++){
		k_j_c = generateSeed(k_chal);
		c[j].j = j;
		c[j].k_j_c = k_j_c;
	}
	
	//execute each challenge w times
	for (i=0;i<size;i++){
		
		//not sure if output of the challenge is a single array or double
		unsigned char * codeword = (unsigned char *) malloc(sizeof(unsigned char)*32*w);
		if (codeword == NULL) {
			fprintf(stderr, "failed to allocate memory for codeword.\n");
			return -1;
		}

		if((v_chal_indices=(int *)malloc(sizeof(int)*v))==NULL){
			fprintf(stderr, "failed to allocate memory for v indices.\n");
			return -1;	
		}

		index = 0;
		for(u=1;u<=w;u++){
			//execute challenge returns Qj : call up function of Rucha
			codeword[index] = execute_challenge(c[i].j, c[i].k_j_c, u, v_chal_indices);
			index=index+32;
		}

		// inner code decoding
		inner_decoding(d1,codeword,v_chal_indices); 
	}
	
	//array to store byte location of the erasures
	int * erasureLocs = (int *) malloc(sizeof(int)*t);
	int e_index=0;
	
	for (i=1;i<=t;i++){
		int max_frequency=0;
		int max_index=0;
		
		for(j=0;j<sizeof(d1[i].frequency);j++){
			if(d1[i].frequency[j] > max_frequency){
				max_frequency = d1[i].frequency[j];
				max_index = j;
			}
		}
		
		//check if the location can be corrected or has erasure 
		if(ceil(max_frequency / sizeof(d1[i].frequency)) > (delta+0.5)){
			F[i] = (char *) malloc ( sizeof(char)*sizeof(d1[i].file_blocks[j]));
			strcpy(F[i],d1[i].file_blocks[j]);
		}else{
			erasureLocs[e_index++] = j;
		}
	}
	
	//perform outer decoding
	outer_decoding(F,fp2,prptable,k_ecc_perm,k_ecc_enc);
	
	//compute new mac and verify against old one
	unsigned char originalmac[16];
	int bufLength=16;
	fseek(fp1, 0, SEEK_END);
	long fileLength=ftell(fp1);
	fseek(fp1, fileLength-bufLength, SEEK_SET);
	fread(&originalmac, sizeof(originalmac), 1, fp1);
	
	//compute mac
	unsigned char * newmac; 
	hmac(r_file,newmac,k_mac);
	
	//if verified, print the file. Else output the error
	if (strcmp(newmac,originalmac)==0){
		printf("Your file is recovered");
    		while (fscanf(fp2, "%s", str)!=EOF){
        		printf("%s",str);
		}
	}else{
		printf("Your file can not be recovered.");
		return -1;
	}

	fclose(fp1);	
	fclose(fp2);
	return 0;
	
}

/* 1. decrypt the parity block using key k3
2. decode using key k2
3. unpermute all parity blocks and store
4. for the permutted message, get each of the strip , append it with parity. 
5. deocode using ecc out
6. we will get permutted file
7. unpermute it using pRP table 
*/

/*Input parameters : 
1. original file pointer
2. int array for erasure locations
3. recovered data from challenges
4. prp table
5. keys for permutation and encryption*/

int outer_decoding (FILE* fp,int * erasureLocs,char ** r_data,int* prptable,unsigned char* k_ecc_perm,unsigned char* k_ecc_enc)
{
	int i,j,blocks,fileLen,index;
	
	//find the numebr of blocks in the file
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	if(fileLen % k==0) 
		blocks = fileLen/k;
	else
		blocks = fileLen/k+1;
	char message[k];
	char codeword[n];
	char parity[blocks][d];
	char decodedfile[blocks][k];

	//read all the parity blocks, decrypt and unpermute
	prptable =(int *)malloc(sizeof(int)*blocks);
	prptable = prp(blocks, k_ecc_perm);
	enc_init(k_ecc_enc);
	
	//decrypt parity part
	for (i=0;i<blocks;i++) {
		unsigned char ct[d];

		decrypt(ct,r_data[i+blocks]);
		for(j=0;j<d;j++){
			parity[i][j]=ct[j];
		}
	}

	//get the message and the parity, create codeword and decode it
	for(i=0;i<blocks;i++) {
		index=0;
		for(j=0;j<k;j++) {
			codeword[index++] = r_data[i][j];
		}
		//calculate block index of the parity part
		int original_index =  prptable[i];
		int parity_index = (original_index*32);
		
		for(j=0;j<d;j++){
			codeword[index++]=parity[parity_index][j];
		}
		
		decode_data(codeword,n);
		strcpy(decodedfile[i*k],codeword);
	}
	
	//correct erasures
	unsigned char * r_erasure;
	correct_errors_erasures(r_erasure, sizeof(erasureLocs)*k,sizeof(erasureLocs), erasureLocs);
	
	// write data to the file
	for(i=0;i<blocks;i++){
		int index=0;
		if(erasureLocs[i] == i){
			fwrite(r_erasure,k,1,fp);
			index++;
		}
		else{
			fwrite(r_data[i-index],k,1,fp);
		}
	}
	fclose(fp);
	return 0;

}

void inner_decoding(decoding *d1, char * codeword, int * v_chal_indices){

	int i=0,j=0;
	char block[32];
	
	//erasure should be empty array? 
	int erasure[1];

	//corrected codeword would be in first parameter as well
	correct_errors_erasures (codeword,sizeof(codeword),0,erasure);
	
	//divide decoded message into v blocks from f1 to fv. 
	for(i=0;i<v;i++){
		//divide codeword into 32 byte blocks
		int fi = v_chal_indices[i];
		for(j=0;j<32;j++){
			block[j]=codeword[(i*32)+j];
		}
		
		for(j=0;j<sizeof(d1[fi].file_blocks);j++){
			if(strcmp(d1[fi].file_blocks[j],block)){
				int freq = d1[fi].frequency[j];
				d1[fi].frequency[j] = freq++;			
			}		
		}
		d1[fi].file_blocks[j] = block;
		d1[fi].frequency[j] = 1;	
	}	
}
