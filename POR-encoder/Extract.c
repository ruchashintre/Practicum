#include <stdio.h>
#include <stdlib.h>
#include "eccwrapper.h"
#include "FeistelPRP.h"
#include "encwrapper.h"
#include "keygenwrapper.h"
#include <math.h>

#define alpha 10
#define delta 0.25
#define BLOCK_SIZE 32
#define n 255
#define k 233
#define d 32
#define w 4096/32 
#define v 1024/32 
#define MACSIZE 16
#define n1 64
#define k1 32
#define d1 32
#define n2 64
#define k2 32
#define d2 12

static unsigned long q,t;

//Challenge structure
typedef struct { 
	unsigned long j;
	unsigned char k_j_c[16];
	int u;
}chal;


//decoding file blocks structure
typedef struct {
	char ** file_blocks; 
	int * frequency; 
}decoded_blocks; 

extern unsigned char k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];

int outer_decoding(FILE* temp_fp, FILE *output, decoded_blocks *db);
void inner_decoding(decoded_blocks *db,unsigned char * c_in_codeword, int * v_chal_indices);

int extract(unsigned long input_t, unsigned char * masterkey, char * filename)
{	
	t = input_t;
	q = alpha * t / v;
	decoded_blocks * db;
	unsigned int i,j,u,size,index;
	char * codeword,mac;
	chal * c;
	unsigned char k_j_c[16]; 
	char str[999]; 
	FILE *fp1,*fp2,*temp_fp; 
	char * temp_block;
	// after writing new file, delete old file

	char * r_file="recovered";
	char * temp ="temp";
	unsigned int * v_chal_indices;

	//use master key and call keygen to generate all the keys here.	
	master_keygen(masterkey); // integrate
	
	//open encoded file for reading	
  	if ((fp1 = fopen(filename, "r")) == NULL){
	         printf("couldn't open input file for reading.\n");
	         return -1;
    	}

	//read mac from the end of the old file	
	unsigned char originalmac[16];
	int bufLength=16;
	fseek(fp1, 0, SEEK_END); // read file before
	long fileLength=ftell(fp1);
	fseek(fp1, fileLength-bufLength, SEEK_SET);
	fread(originalmac, sizeof(originalmac), 1, fp1);

	//open output file for writing
	if ((fp2 = fopen(r_file, "w+")) == NULL){
	         printf("couldn't open output file for writing.\n");
	         return -1;
    	}

	//open temp file for writing
	if ((temp_fp = fopen(temp, "w+")) == NULL){
	         printf("couldn't open temperory file for writing.\n");
	         return -1;
    	}

	//allocate memory for d1
	db = (decoded_blocks *) malloc (sizeof(decoded_blocks)*alpha*t);  
	if(db == NULL) {
		printf("failed to allocate memory for d.\n");
		return -1;
	}

	//total number of challenges		
	size = alpha*(t/v); 

	//allocate memory for the challenge set
	if ((c = (chal *)malloc(sizeof(chal)*size))== NULL) {
		fprintf(stderr, "failed to allocate memory for challenges.\n");
		return -1;
	}
	
	// populate challenge set
	keygen_init();
	seeding(k_chal);	
	for (j=0;j<size;j++){
		keygen(kjc, 16);
		c[j].j = j;
		c[j].k_j_c = k_j_c;		
	}
	
	//execute each challenge w times
	for (i=0;i<size;i++){
		
		unsigned char * codeword = (unsigned char *) malloc(sizeof(unsigned char)*32*w);
		if(codeword== NULL){
			fprintf(stderr, "failed to allocate memory for codeword.\n"); 
			return -1;
		}

		if((v_chal_indices=(int *)malloc(sizeof(int)*v))==NULL){
			fprintf(stderr, "failed to allocate memory for v indices.\n");
			return -1;	
		}

		index = 0;
		for(u=0;u<w;u++){
			codeword[index] = execute_challenge(c[i].j, c[i].k_j_c, u, v_chal_indices);
			index=index+32;
		}

		// inner code decoding
		inner_decoding(db,codeword,v_chal_indices); 

		//free the memory
		free(codeword);
		free(v_chal_indices);

		//delete old file
		fclose(fp1);	
		remove(filename);
	}
	
	for (i=1;i<=t;i++){
		int max_frequency=0;
		int max_index=0;
		
		for(j=0;j<sizeof(db[i].frequency);j++){
			if(db[i].frequency[j] > max_frequency){
				max_frequency = db[i].frequency[j];
				max_index = j;
			}
		}
		
		//check if the location can be corrected or has erasure 
		if(ceil(max_frequency / sizeof(db[i].frequency)) > (delta+0.5)){
			fwrite(db[i].file_blocks[j],sizeof(db[i].file_blocks[j]),1,temp_fp);
		}else{
			//where to get the index from
			db[i].file_blocks[0]=NULL;
	
			//-1 indicating erasure
			db[i].frequency[0]=-1;
		}
	}
	
	//perform outer decoding
	outer_decoding(temp_fp,fp2,db);
	
	//compute mac
	unsigned char * newmac; 
	hmac(r_file,newmac,k_mac);
	
	//if verified, print the file. Else output the error
	int flag=1;	
	for(i=0;i<MACSIZE;i++){
		if(newmac[i]!=originalmac[i]){	
			flag=0;
			break;
		}
	}
	if (flag==1){
		printf("Your file is recovered");
    		while (fscanf(fp2, "%s", str)!=EOF){
        		printf("%s",str);
		}
	}else{
		printf("Your file can not be recovered.");
		return -1;
	}

	fclose(fp2);
	return 0;
}

// Procecdure for OUTER DECODING
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

void reverseprp(int stripes, int * prp_table, int * reverse_prp_table)
{
	int i=0;
	for(i=0;i<sizeof(prp_table);i++){
		int num=prp_table[i];
		reverse_prp_table[num]=i;	
	}
}

int outer_decoding(FILE* temp_fp, FILE *output, decoded_blocks *db)
{
	int i,j,stripes,fileLen,index,m;
	int * prp_table, *reverse_prp_table;
	char message[k];
	char codeword[n];
	char parity[stripes][d]; 
	char decodedfile[stripes][k];
	int erasure[1];

	//find the number of stripes in the file 
	fseek(temp_fp,0,SEEK_END);
	fileLen = ftell(temp_fp);
	if(fileLen % k==0) 
		stripes = fileLen/k; 
	else
		stripes = fileLen/k+1;	

	//call prp
	prp_table =(int *)malloc(sizeof(int)*stripes);
	reverse_prp_table = (int *)malloc(sizeof(int)*stripes);
	
	//perform reverse prp
	prp_table = prp(stripes, k_ecc_perm);
	reverseprp(stripes,prp_table,reverse_prp_table);
	free(prp_table);

	enc_init(k_ecc_enc);
	
	//decrypt parity part
	rewind(temp_fp);
	
	///number of parity blocks = stripes/2
	//read parity parts directly
	// not sure about block length : CHECK WITH JIE,Rucha
	//can we do directly like this?
	int parity_start = fileLen-(stripes/2)*d;
	fseek(temp_fp,parity_start,SEEK_SET);

	for (i=0;i<stripes/2;i++) {
		unsigned char ct[d];
		unsigned char pt[d];	
		
		fread(pt,sizeof(pt),1,temp_fp);
		
		decrypt(ct,pt,sizeof(ct)); 
		for(j=0;j<d;j++){
			parity[i][j]=ct[j];
		}
	}

	//get the message and the parity, create codeword and decode it
	rewind(temp_fp);
	
	for(i=0;i<stripes/2;i++) {
		index=0;
		unsigned char pt[k];

		fread(pt,sizeof(pt),1,temp_fp);

		for(j=0;j<k;j++) {
			codeword[index++] = pt[j]; 
		}
		//calculate block index of the parity part
		int parity_index = (reverse_prp_table[i]*32);
		
		for(j=0;j<d;j++){
			codeword[index++]=parity[parity_index][j];
		}
		
		// how to get erasures here from Di? ASK JIERUCAA
		correct_errors_erasures(codeword,n,0,erasure); 
	
		for(m=0;m<k;m++){
			decodedfile[i*k][m]=codeword[m];
		}
	}
	
	// write data to the file by applying second level of permutation
	for(i=0;i<stripes;i++){
		int fileindex=reverse_prp_table[i];
		int block_start_index= i*32;
		int block_end_index=(i+1)*32;
		unsigned char ct[32];
		for(j=i*block_start_index;j<block_end_index;j++){
			ct[j]=decodedfile[j];
		}
		fwrite(ct,sizeof(ct),1,output);
	}
	fclose(output);
	delete(temp_fp);
	return 0;
}

//concatenated decoding
void inner_decoding(decoded_blocks *db,unsigned char * c_in_codeword, int * v_chal_indices){ 

	unsigned char c_in_message[v*k2],temp[n2],c_out_codeword[n1],c_out_message[v*k1];
	int c_index=0,m_index=0,i,j,p,m,index=0;
	int erasure[1];
	
	// cin decoding
	for(j=0;i<sizeof(c_in_codeword);j++){
		for(i=0;i<n2;i++){
			temp[i]=c_in_codeword[c_index++];
		}
		correct_errors_erasures(temp,n2,0,erasure);
		for(i=0;i<k2;i++){
			c_in_message[m_index++]=temp[k2];	
		}
	}

	//cout decoding: get the file and parity part
	c_index=0;
	for(i=0;i<m_index/2;i++){
		index=0;
		//create codeword
		//copy message part
		for(j=0;j<k1;j++){
			c_out_codeword[index++]=c_in_message[j];		
		}

		//copy parity part codeword
		p = (m_index/2) + (i*d1);
		for(j=0;j<d1;j++){
			c_out_codeword[index++]=c_in_message[j+p];		
		}	
		
		correct_errors_erasures(c_out_codeword,n1,0,erasure);
		
		for(j=0;j<k1;j++){
			c_out_message[c_index++]=c_out_codeword[j];
		}
	}
	
	//c_out_message contains v decoded blocks
	//divide decoded message into v blocks from f1 to fv. 
	for(i=0;i<v;i++){
		//divide codeword into 32 byte blocks
		int fi = v_chal_indices[i];
		char block[32];
		for(j=0;j<32;j++){
			block[j]=c_out_message[(i*32)+j];
		}
		
		//check if similar codeword was already decoded and present in Di
		//if yes, just increase the frequency
		int notfound=1;
		for(j=0;j<sizeof(db[fi].file_blocks);j++){
			int flag=1;
			for(m=0;m<32;m++){
				if(db[fi].file_blocks[j][m]!=block[m]){
					flag=0;
					break;
				}		
			}
			if(flag){
				int freq = db[fi].frequency[j];
				db[fi].frequency[j] = freq++;	
				notfound=0;		
			}		
		}

		//if not found, add it in di		
		if(notfound){
			for(m=0;m<32;m++){
				db[fi].file_blocks[j][m] = block[m];		
			}
			db[fi].frequency[j] = 1;
		}	
	}	
}
