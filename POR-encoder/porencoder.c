//#include "keygenwrapper.h"
#include "eccwrapper.h"
#include "FeistelPRP.h"

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
	int i;
	int k = 223, n = 255;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	if(fileLen % k==0) 
		blocks = fileLen/k;
	else
		blocks = fileLen/k+1;
	char message[223];
	char codeword[blocks][n];
	int readLen = 512*1024*1024;
	char * buf = malloc(sizeof(buf)*readLen);
	int filecounter = 0;
    int blockcounter = 0;
	int endIndicator = readLen;
	while (!feof(fp))
	{
		fread(buf, readLen, 1, fp);
		int index = prptable[counter];
		int startind = 32*index;
		int endind = 32*index+31;
		if (startind<=readLen && endind<=readLen)
		{
			for (i=0;i<32;i++)
			{
				message[blockcounter++] = buf[startind+i];
			}
		}
	}
}

int main(int argc, char* argv[])
{
	int i;
	FILE* fp = fopen(argv[1],"a+");
	int* prptable,s;
	unsigned char mac[MAXBLOCKSIZE],
	k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_mac[16];
	int n,k,q,v,w,u;
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
	keygen(k_mac,16);
	printf("key for MAC computation: ");
	displayCharArray(k_mac);	
	
	int blocks = blockize(fp);
	
	printf("\nComputing file's MAC...\n");
	hmac(argv[1],mac,k_mac);
	printf("\nMAC = ");
	displayCharArray(mac);	
	
	prptable = malloc(sizeof(int)*blocks);
	prptable = prp(blocks, k_file_perm);
	
	n = 255;
	k = 223;
	ecc_initialize (n, k, k-n);
	inc_encoding (fp,prptable,k_ecc_perm,k_ecc_enc);
}


