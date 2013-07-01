#include "keygenwrapper.h"
#include "eccwrapper.h"
#include "FeistelPRP.h"
#include "encwrapper.h"

#define v 1024/32
#define w 4096/32
#define n 255
#define k 223
#define n1 64
#define k1 32
#define n2 64
#define k2 32

static unsigned long q,t;
extern unsigned char k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];

typedef struct {
	unsigned long s[v];
	unsigned long u;
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

void displayCharArray(unsigned char* out,int len)
{
	int i;
	for (i = 0;i < len; i++) {
		printf("%02x", out[i]);
	}
	printf("\n");
}

int blockize(FILE* fp)
{
	unsigned long fileLen;
	unsigned int file_blocks;
	unsigned int i;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	printf("\nfile size: %lu\n",fileLen);
	if(fileLen % 32==0) {
		file_blocks = fileLen/32;
		printf("There are %d 32-byte blocks\n",file_blocks);
	}
	else
	{
		file_blocks = fileLen/32+1;
		int padding = 32 - fileLen % 32;
		unsigned char paddingBytes[padding];
		for (i=0;i<padding;i++)
			paddingBytes[i] = 0;
		fwrite(paddingBytes,padding,1,fp);
		printf("After padding %d zeros, there are %d 32-byte blocks\n",padding,file_blocks);
	}
	return file_blocks;
}

int inc_encoding (FILE* fp,int* prptable)
{
	printf("\nIncremental encoding starts...\n");
	int i,j,enc_blocks,d=n-k;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	if(fileLen % k==0) 
		enc_blocks = fileLen/k;
	else
		enc_blocks = fileLen/k+1;
	char message[k];
	char codeword[n];
	char code[enc_blocks][d];
	int readLen = 512*1024*1024;
	char * buf = malloc(sizeof(buf)*readLen);
	int filecounter = 0;
    int blockcounter = 0;
	int round = 0;
	while (!feof(fp))
	{
		size_t br = fread(buf, readLen, 1, fp);
		filecounter = filecounter + br;
		for(i=0;i<enc_blocks;i++) {
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
	prptable = malloc(sizeof(int)*enc_blocks);
	printf("\nSRF PRP for the outer layer ECC...\n");
	prptable = prp(enc_blocks, k_ecc_perm);
	enc_init(k_ecc_enc);

	for (i=0;i<enc_blocks;i++) {
		unsigned char ct[d];
		encrypt(ct,code[prptable[i]],sizeof(code[prptable[i]]));
		fwrite(ct,1,d,fp);
	}
	t = t+enc_blocks;
	printf("\nIncremental encoding finishes...\n");
	return 0;
}

void concat_encode(unsigned char * message,unsigned char* codeword) {
	unsigned char tmp_code[v*32*n1/k1],stripe[k1],stripe_code[n1];
	int index,i,j;
	
	for (index=0;index<v*32;index++) {
		tmp_code[index] = message[index];
	}
	for (i=0;i<v;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = message[i*k1+j];
		}
		initialize_ecc();
		encode_data(stripe,k1,stripe_code);
		for (j=0;j<n1-k1;j++) {
			tmp_code[index] = stripe_code[k1+j];
			index++;
		}
	}
	index = 0;
	for (i=0;i<v*n1/k1;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = tmp_code[i*k2+j];
		}
		encode_data(stripe,k2,stripe_code);
		for (j=0;j<n2;j++) {
			codeword[index] = stripe_code[j];
			index++;
		}
	}
}

int precompute_response(FILE* fp, Chal * c,char * key) {
	unsigned char message[v*32];
	unsigned char codeword[w*32];
	char uth[32];
	char ct[32];
	int i,j,p;
	enc_init(key);
	for (j=0;j<q;j++) {
	printf("Precomputation for challenges No.%d\n",j);
		int index = 0;
		for (i=0;i<v;i++) {
			printf("s[%d]=%lu\n",i,c[j].s[i]);
			fseek(fp,c[j].s[i]*32,SEEK_SET);
			unsigned char buffer[32];
			fread(buffer, 32, 1, fp);
			for(p=0;p<32;p++) {
				message[index] = buffer[p];
				index++;
			}
			fflush(stdout);
		}

		concat_encode(message,codeword);
		for (i=0;i<32;i++) {
			uth[i] = codeword[32*c[j].u+i];
		}
		printf("u=%lu\n",c[j].u);
		encrypt(ct,uth,sizeof(uth));
		printf("Precomputation for response No.%d\n",j);
		displayCharArray(ct,32);
		fwrite(ct,1,32,fp);
		fflush(stdout);
	}
}

int main(int argc, char* argv[])
{
	int i;
	FILE* fp = fopen(argv[1],"a+");
	int* prptable;
	unsigned char mac[MAXBLOCKSIZE];
	//k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	//k_chal[16],k_ind[16],k_enc[16],k_mac[16];
	
	master_keygen(argv[2]);
	//keygen(k_file_perm,16);
	printf("key for file permutation: ");
	displayCharArray(k_file_perm,16);
	//keygen(k_ecc_perm,16);
	printf("key for ecc permutation: ");
	displayCharArray(k_ecc_perm,16);
	//keygen(k_ecc_enc,16);

	printf("key for ecc encryption: ");
	displayCharArray(k_ecc_enc,16);
	//keygen(k_chal,16);
	printf("key for challenge generation: ");
	displayCharArray(k_chal,16);
	//keygen(k_ind,16);
	printf("key for random index generation: ");
	displayCharArray(k_ind,16);
	//keygen(k_enc,16);
	printf("key for response encryption: ");
	displayCharArray(k_enc,16);
	//keygen(k_mac,16);
	printf("key for MAC computation: ");
	displayCharArray(k_mac,16);	
	
	int blocks = blockize(fp);
	t = blocks;
	
	printf("\nComputing file's MAC...\n");
	hmac(argv[1],mac,k_mac);
	printf("\nMAC = ");
	displayCharArray(mac,16);	
	
	prptable = malloc(sizeof(int)*blocks);
	printf("\nSRF PRP for the entire file...\n");
	prptable = prp(blocks, k_file_perm);
	//for(i=0;i<blocks;i++)
	//	printf("%d -> %d\n",i,prptable[i]);

	initialize_ecc();
	inc_encoding(fp,prptable);
	
	printf("\nFile blocks after outer layer encoding: %lu\n",t);

	q = 100;
	printf("\nPrecomputation for %lu challenges and responses\n",q);
	Chal c[q];
	int j,p;
	keygen_init();
	seeding(k_chal);
	unsigned char * kjc[q];
	for(j=0;j<q;j++) {
		kjc[j] = malloc(16*sizeof(unsigned char *));
		keygen(kjc[j], 16);
		printf("display kjc for j=%d\n",j);
		displayCharArray(kjc[j],16);
	}
	for(j=0;j<q;j++) {
		keygen_init();
		seeding(kjc[j]);
		for(p=0;p<v;p++) {
			unsigned long randomIndex;
			char rand[8];
			keygen(rand, 8);
			printf("display rand for j=%d,v=%d\n",j,p);
			randomIndex = *(unsigned long *)rand;	
			c[j].s[p] = randomIndex % t;
			printf("display random index for j=%d,v=%d: %lu\n",j,p,c[j].s[p]);
		}
	}
	keygen_init();
	seeding(k_ind);	
	for(j=0;j<q;j++) {
		unsigned long randomIndex;
		char rand[8];
		keygen(rand, 8);
		randomIndex = *(unsigned long *)rand;	
		c[j].u = randomIndex % w;
		printf("display rand for j=%d,u=%lu\n",j,c[j].u);
	}
	printf("Precomputation for challenges finishes\n",q);
	precompute_response(fp,c,k_enc);
	printf("Precomputation for responses finishes\n",q);
	
	printf("\nAppend MAC to the end of the file...\n",q);
	fwrite(mac,1,16,fp);
	fclose(fp);
	printf("\nPOR encoding done\n",q);
}



