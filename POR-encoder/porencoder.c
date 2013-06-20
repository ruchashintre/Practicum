//#include "keygenwrapper.h"
#include "eccwrapper.h"
#include "FeistelPRP.h"
#include "encwrapper.h"

#define v 1024/32
#define w 4096/32
#define n 255
#define k 223

int q,t;

typedef struct {
	int s[v];
	int u;
} Chal;

int hmac(char* filename,unsigned char* dst,unsigned char* key)
{
	int idx, err;
	hmac_state hmac;
	unsigned long dstlen;
	if (register_hash(&sha1_desc)==-1) {
		printf("error registering SHA1\n");
		return -1;
	}
	idx = find_hash("sha1");

	dstlen = sizeof(dst);

	if ((err = hmac_file(idx,filename,key,16,dst,&dstlen))!=CRYPT_OK) {
		printf("Error hmac: %s\n",error_to_string(err));
		return -1;
	}
    
    printf("hmac complete\n");
    return 0;
}

void displayCharArray(unsigned char* out)
{
	int i;
	for (i = 0; (i < sizeof(out)); i++) {
		printf("%02x", out[i]);
	}
	printf("\n");
}

int blockize(FILE* fp)
{
	unsigned long fileLen;
	unsigned int blocks;
	unsigned int i;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	printf("\nfile size: %lu\n",fileLen);
	if(fileLen % 32==0) {
		blocks = fileLen/32;
		printf("There are %d 32-byte blocks\n",blocks);
	}
	else
	{
		blocks = fileLen/32+1;
		int padding = 32 - fileLen % 32;
		unsigned char paddingBytes[padding];
		for (i=0;i<padding;i++)
			paddingBytes[i] = 0;
		fwrite(paddingBytes,padding,1,fp);
		printf("After padding %d zeros, there are %d 32-byte blocks\n",padding,blocks);
	}
	return blocks;
}

int inc_encoding (FILE* fp,int* prptable,unsigned char* k_ecc_perm,unsigned char* k_ecc_enc)
{
	int i,j,blocks,d=n-k;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	if(fileLen % k==0) 
		blocks = fileLen/k;
	else
		blocks = fileLen/k+1;
	char message[k];
	char codeword[n];
	char code[blocks][d];
	int readLen = 512*1024*1024;
	char * buf = malloc(sizeof(buf)*readLen);
	int filecounter = 0;
    int blockcounter = 0;
	int round = 0;
	while (!feof(fp))
	{
		size_t br = fread(buf, readLen, 1, fp);
		filecounter = filecounter + br;
		for(i=0;i<blocks;i++) {
			for(j=0;j<k;j++) {
				int index = i*k+j;
				int block_index = index/32;
				int byte_index = index%32;
				if (block_index*32+byte_index>=fileLen) {
					int a;
					for(a=j;a<k;a++)
						message[a]=0;
					break;
				}
				int file_index = prptable[block_index]*32+byte_index;
				if(file_index<=filecounter)
					message[j] = buf[file_index-round*readLen];
				else 
					message[j] = 0;
			}
			encode_data(message,k,codeword);
			for(j=0;j<d;j++)
				code[i][j] = code[i][j] ^ codeword[k+j];
		}
		round = round + 1;
	}
	prptable = malloc(sizeof(int)*blocks);
	prptable = prp(blocks, k_ecc_perm);
	enc_init(k_ecc_enc);
	for (i=0;i<blocks;i++) {
		unsigned char ct[d];
		encrypt(ct,code[prptable[i]]);
		fwrite(ct,1,d,fp);
	}
	t = t+blocks;
	return 0;
}

int precompute_response(FILE* fp, Chal * c,char * key) {
	char message[v*32];
	char codeword[w*32];
	char uth[32];
	char ct[32];
	int i,j;
	for (j=0;j<q;j++) {
		for (i=0;i<v;i++) {
			fseek(fp,i*32,SEEK_SET);
			unsigned char buffer[32];
			fread(buffer, 32, 1, fp);
			strcat(message,buffer);
		}
		concat_encode(message,v,codeword);
		for (i=0;i<32;i++)
			uth[i] = codeword[32*c[j].u+i];
		enc_init(key);
		encrypt(ct,uth);
		fwrite(ct,1,32,fp);
	}
}

int main(int argc, char* argv[])
{
	int i;
	FILE* fp = fopen(argv[1],"a+");
	int* prptable,s;
	unsigned char mac[MAXBLOCKSIZE],
	k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];

	keygen_init();
	seeding(argv[2]);
	keygen(k_file_perm,16);
	printf("key for file permutation: ");
	displayCharArray(k_file_perm);
	keygen(k_ecc_perm,16);
	printf("key for ecc permutation: ");
	displayCharArray(k_ecc_perm);
	keygen(k_ecc_enc,16);
	printf("key for ecc encryption: ");
	displayCharArray(k_ecc_enc);
	keygen(k_chal,16);
	printf("key for challenge generation: ");
	displayCharArray(k_chal);
	keygen(k_ind,16);
	printf("key for random index generation: ");
	displayCharArray(k_ind);
	keygen(k_enc,16);
	printf("key for response encryption: ");
	displayCharArray(k_enc);
	keygen(k_mac,16);
	printf("key for MAC computation: ");
	displayCharArray(k_mac);	
	
	int blocks = blockize(fp);
	t = blocks;
	
	printf("\nComputing file's MAC...\n");
	hmac(argv[1],mac,k_mac);
	printf("\nMAC = ");
	displayCharArray(mac);	
	
	prptable = malloc(sizeof(int)*blocks);
	prptable = prp(blocks, k_file_perm);
	

	initialize_ecc();
	inc_encoding(fp,prptable,k_ecc_perm,k_ecc_enc);
	
	q = 2;
	Chal c[q];
	c[0].s = {5,163,1234,23,412,51,61,1234,
	716,247,3568,3145,356835,1354,1457,356,
	3576,2345,1234,478,456,4,587,46789,
	27682,765,2345,8476,2456,3568,2346,6};
	c[0].u = 4;
	c[1].s = {2456,12345,1345,3425,34,3,6,234,
	76,65,765,734,8476,657,4265,63,2345,5,
	27682,765,2345,8476,2456,3568,2346,16,
	3576,2345,1234,478,456,4,587,46789};
	c[1].u = 20;
	precompute_response(fp,c,k_enc);
	
	fwrite(mac,1,16,fp);
	fclose(fp);
}



