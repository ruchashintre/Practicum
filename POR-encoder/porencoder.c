#include "por.h"

static unsigned long t;
extern unsigned char k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];
static double totalTime,totalStartTime, totalEndTime;
static double readTime,readStartTime, readEndTime;
static double prpTime,prpStartTime, prpEndTime;
static double eccTime,eccStartTime, eccEndTime;
static double macTime,macStartTime, macEndTime;
static double chalTime,chalStartTime, chalEndTime;
static double write1Time,write1StartTime, write1EndTime;
static double write2Time,write2StartTime, write2EndTime;
static double encTime,encStartTime,encEndTime;
static clock_t startTime,endTime;
static struct timespec start, finish;

typedef struct {
	unsigned long s[v];
	unsigned int u;
} Chal;

// blockize the file, padding 0 if not divisible by BLOCK_SIZE
int blockize(FILE* fp)
{
	unsigned long fileLen;
	unsigned int file_blocks;
	unsigned int i;
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp); // get file length
	printf("\nfile size: %lu\n",fileLen);

	if(fileLen % BLOCK_SIZE==0) {
		// file divisible by block size
		file_blocks = fileLen/BLOCK_SIZE;
		printf("There are %d blocks\n",file_blocks);
	}
	else
	{
		// if not divisible, padding 0 at the end
		file_blocks = fileLen/BLOCK_SIZE+1;
		int padding = BLOCK_SIZE - fileLen % BLOCK_SIZE;
		unsigned char paddingBytes[padding];
		for (i=0;i<padding;i++)
			paddingBytes[i] = 0;
        write1StartTime = getCPUTime();
		fwrite(paddingBytes,padding,1,fp);
        write1Time += getCPUTime() - write1StartTime;
		printf("After padding %d zeros, there are %d blocks\n",padding,file_blocks);
	}
	return file_blocks;
}

// outer layer ECC using incremental encoding
int inc_encoding (FILE* fp,int* prptable)
{
	printf("\nIncremental encoding starts...\n");
	int i,j,enc_blocks,d=n-k;
	// get file length
	fseek(fp,0,SEEK_END);
	fileLen = ftell(fp);
	// divide by message length k, get number of encoding blocks
	if(fileLen % k==0) 
		enc_blocks = fileLen/k;
	else
		enc_blocks = fileLen/k+1;
	printf("There are %d encoding blocks\n",enc_blocks);
	unsigned char message[k];
	unsigned char codeword[n];
	unsigned char ** code; // used to store parity part
	
	long filecounter = 0;
	int blockcounter = 0;
	int round = 0;

	// code is enc_blocks * d
	code = (unsigned char **) malloc(enc_blocks*sizeof(unsigned char *));  
	for (i = 0; i < enc_blocks; i++) {
   	code[i] = (unsigned char *) malloc(d*sizeof(unsigned char)); 
   	int ii;
   	for (ii=0;ii<d;ii++)
   		code[i][ii]=0; 
   	}
   	

   rewind(fp);
	while (!feof(fp))
	{
		unsigned char * buf; 
		if ((buf = malloc(sizeof(unsigned char)*readLen))==NULL) {
			printf("malloc error: inc_encoding\n");
			exit(1);
		}
		// incremental encoding, read reaLen each time
		readStartTime = getCPUTime();
		clock_gettime(CLOCK_MONOTONIC, &start);
		printf("max read in %d bytes\n",readLen);
		size_t br = fread(buf, 1, readLen, fp);
		printf("Read in %lu bytes\n",br);
		fflush(stdout);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		double addTime = finish.tv_sec - start.tv_sec;
		addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
		readTime += getCPUTime() - readStartTime+addTime;
		// keep a counter to know where the file pointer up to
		filecounter = filecounter + br;
		if (br!=0) {
			printf("round %d\n",round);
			printf("filecounter = %lu\n",filecounter);
			for(i=0;i<enc_blocks;i++) {
				for(j=0;j<k;j++) {
					// for each byte in each message, compute index
					int index = i*k+j;
					// get block and byte index
					int block_index = index/BLOCK_SIZE;
					int byte_index = index%BLOCK_SIZE;
					// if reach the end, padding 0s
					if (index>=fileLen) {
						int a;
						for(a=j;a<k;a++)
							message[a]=0;
						break;
					}
					// compute the PRPed index in the file
					unsigned long file_index = prptable[block_index]*BLOCK_SIZE+byte_index;
					
					// check if this byte is read in the memory or not, copy if yes, put 0 otherwise
					if(file_index<filecounter && file_index>=(filecounter-br)) {
						unsigned long newind = file_index-filecounter+br;
						message[j] = buf[newind];
					}
					else 
						message[j] = 0;
				}
				//printf("msg for block %d: ",i);
				//displayCharArray(message,k);
				// do a partial encoding on the message
				encode_data(message,k,codeword);
				// concatenate with previous code to get a whole
				/*printf("code for block %d: ",i);
				displayCharArray(codeword,n);
				printf("parity (before) for block %d: ",i);
				displayCharArray(code[i],n-k);*/
				for(j=0;j<d;j++)
					code[i][j] = code[i][j] ^ codeword[k+j];
				//printf("parity for block %d: ",i);
				//displayCharArray(code[i],n-k);
				//printf("\n");
			}
			round = round + 1;
		}
		free(buf);
	}
	/*// ------------- for debugging
	unsigned char a[fileLen],r[fileLen];
	unsigned char newc[n],newm[k];
	rewind(fp);
	fread(a, 1, fileLen, fp);
	printf("original:\n");
	for (i=0;i<fileLen;i++) {
		printf("%02x",a[i]);
	}
	printf("\n");
	for (i=0;i<fileLen/32;i++) {
		for (j=0;j<32;j++) {
			r[i*32+j] = a[prptable[i]*32+j];
		}
	}
	printf("prped:\n");
	for (i=0;i<fileLen;i++) {
		printf("%02x",r[i]);
	}
	printf("\n");
	for (i=0;i<enc_blocks;i++) {
		printf("parity part %d: ",i);
		displayCharArray(code[i],d);

		unsigned char newcode[n];
		int iii;
		int ii;
		for(ii=0;ii<k;ii++) {
			if (i*k+ii>=fileLen)
				break;
			newcode[ii] = r[i*k+ii];
			newm[ii] = r[i*k+ii];
		}
		if (i==enc_blocks-1) {
			for(ii=0;ii<k-fileLen%k;ii++){
				newm[fileLen%k+ii]=0;
				newcode[fileLen%k+ii] = 0;
			}
		}
		encode_data(newm,k,newc);
		printf("actual code %d: ",i);
		displayCharArray(newc,n);
		for(iii=0;iii<d;iii++) {
			newcode[k+iii] = code[i][iii];
		}
		newcode[0] = 99;
		printf("whole code %d: ",i);
		displayCharArray(newcode,n);
		decode_data(newcode, n);
		int erasure[1];
		int syn = check_syndrome ();
		printf("syndrome: %d\n",syn);
		if (syn != 0) {
			correct_errors_erasures(newcode,n,0,erasure);
		}
		printf("decode %d: ",i);
		displayCharArray(newcode,n);
	}
	//--------------- for debugging */
	free(prptable);
	prptable = NULL;
	// perform another PRP for parity part
	prptable = malloc(sizeof(int)*(enc_blocks));
	printf("\nSRF PRP for the outer layer ECC...\n");
   prpStartTime = getCPUTime();
	prptable = prp(enc_blocks, k_ecc_perm);
   prpTime += getCPUTime() - prpStartTime;
   
   // encrypt parity part and append to the file with PRPed order
	enc_init(k_ecc_enc);
	for (i=0;i<enc_blocks;i++) {
		unsigned char ct[d];

    	clock_gettime(CLOCK_MONOTONIC, &start);
    	encStartTime = getCPUTime();
		encrypt(ct,code[prptable[i]],sizeof(ct));
		clock_gettime(CLOCK_MONOTONIC, &finish);
		double addTime = finish.tv_sec - start.tv_sec;
		addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
		encTime += getCPUTime()-encStartTime+addTime;

		//printf("encrypted for %d: ",i);
		//displayCharArray(ct,sizeof(ct));
		//unsigned char pt[d];
		//decrypt(ct,pt,sizeof(ct));
		//printf("decrypted for %d: ",i);
		//displayCharArray(pt,sizeof(ct));
    	clock_gettime(CLOCK_MONOTONIC, &start);
		write1StartTime = getCPUTime();
		fwrite(ct,d,1,fp);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		addTime = finish.tv_sec - start.tv_sec;
		addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
    	write1Time += getCPUTime()-write1StartTime;
	}
	// update t for later challenge computation
	t = t+enc_blocks;
	printf("\nIncremental encoding finishes...\n");
	free(prptable);
	for (i = 0; i < enc_blocks; i++){  
   	free(code[i]);  
	}  
	free(code); 
	return 0;
}

// concatenate two RS code for encoding
void concat_encode(unsigned char * message,unsigned char* codeword) {
	unsigned char tmp_code[v*BLOCK_SIZE*n1/k1],stripe[k1],stripe_code[n1];
	int index,i,j;
	// read in the message part
	for (index=0;index<v*BLOCK_SIZE;index++) {
		tmp_code[index] = message[index];
	}
	// outer encoding on the message
	for (i=0;i<v;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = message[i*k1+j];
		}
		initialize_ecc();
		// encode for each outer stripe
		encode_data(stripe,k1,stripe_code);
		// append parity at the end
		for (j=0;j<n1-k1;j++) {
			tmp_code[index] = stripe_code[k1+j];
			index++;
		}
	}
	index = 0;
	// perform inner encoding
	for (i=0;i<v*n1/k1;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = tmp_code[i*k2+j];
		}
		// encode for each inner stripe
		encode_data(stripe,k2,stripe_code);
		for (j=0;j<n2;j++) {
			codeword[index] = stripe_code[j];
			index++;
		}
	}
}


int precompute_response(FILE* fp, Chal * c,char * key) {

	unsigned char message[v*BLOCK_SIZE];
	unsigned char codeword[w*BLOCK_SIZE];
	char uth[BLOCK_SIZE];
	char ct[BLOCK_SIZE];
	int i,j,p;
	enc_init(key);
    // for each of the challenge
	for (j=0;j<q;j++) {

		int index = 0;
		for (i=0;i<v;i++) {
            // read in the random indexed blocks
            fseek(fp,c[j].s[i]*BLOCK_SIZE,SEEK_SET);
			unsigned char buffer[BLOCK_SIZE];
            readStartTime = getCPUTime();
            clock_gettime(CLOCK_MONOTONIC, &start);
			fread(buffer, BLOCK_SIZE, 1, fp);
            clock_gettime(CLOCK_MONOTONIC, &finish);
            double addTime = finish.tv_sec - start.tv_sec;
            addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
            readTime += getCPUTime() - readStartTime+addTime;
			for(p=0;p<BLOCK_SIZE;p++) {
				message[index] = buffer[p];
				index++;
			}
			fflush(stdout);
		}
        // perform a concatenated encoding
		concat_encode(message,codeword);
		for (i=0;i<BLOCK_SIZE;i++) {
            // get the u-th symbol
			uth[i] = codeword[BLOCK_SIZE*c[j].u+i];
		}
		clock_gettime(CLOCK_MONOTONIC, &start);
		encStartTime = getCPUTime();
        // encrypt the response and append at the end
		encrypt(ct,uth,sizeof(uth));
		clock_gettime(CLOCK_MONOTONIC, &finish);
		double addTime = finish.tv_sec - start.tv_sec;
		addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
		encTime += getCPUTime()-encStartTime+addTime;

        write2StartTime = getCPUTime();
		clock_gettime(CLOCK_MONOTONIC, &start);
		fwrite(ct,BLOCK_SIZE,1,fp);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		addTime = finish.tv_sec - start.tv_sec;
		addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
        write2Time+=getCPUTime()-write2StartTime+addTime;

	}
}

int main(int argc, char* argv[])
{
    totalStartTime = getCPUTime();
    printf("%lf\n",totalStartTime);
	int i;
	FILE* fp = fopen(argv[1],"a+b");
	if (fp==NULL) {
		printf("fopen error: cannot open file\n");
		exit(1);
	}
	int* prptable;
	unsigned char mac[MAXBLOCKSIZE];
    unsigned long fileLen1,fileLen2;
    // get the file size
	fseek(fp,0,SEEK_END);
	fileLen1 = ftell(fp);
    // generate keys
	master_keygen(argv[2]);

	printf("key for file permutation: ");
	displayCharArray(k_file_perm,16);

	printf("key for ecc permutation: ");
	displayCharArray(k_ecc_perm,16);

	printf("key for ecc encryption: ");
	displayCharArray(k_ecc_enc,16);

	printf("key for challenge generation: ");
	displayCharArray(k_chal,16);

	printf("key for random index generation: ");
	displayCharArray(k_ind,16);

	printf("key for response encryption: ");
	displayCharArray(k_enc,16);

	printf("key for MAC computation: ");
	displayCharArray(k_mac,16);	
	
    // blockize the file
	int blocks = blockize(fp);
	t = blocks;
	fclose(fp);
    fp = fopen(argv[1],"a+b");
    
    // computing MAC
	printf("\nComputing file's MAC...\n");
		printf("\nmac size %lu\n",sizeof(mac));
    macStartTime = getCPUTime();
	hmac(argv[1],mac,k_mac);
    macTime = getCPUTime() - macStartTime;
	printf("\nMAC = ");
	displayCharArray(mac,16);	
	
    // perform a file level PRP
	printf("\nSRF PRP for the entire file...\n");
    prpStartTime = getCPUTime();
	prptable = prp(blocks, k_file_perm);
    prpTime += getCPUTime() - prpStartTime;
    eccStartTime = getCPUTime();
	initialize_ecc();
	inc_encoding(fp,prptable);
    eccTime = getCPUTime() - eccStartTime - readTime;
	
	printf("\nFile blocks after outer layer encoding: %lu\n",t);

    // precompute q challenge and responsess
	printf("\nPrecomputation for %d challenges and responses\n",q);
    chalStartTime = getCPUTime();
	Chal c[q];
	int j,p;
    // use k_chal to generate kjc
	keygen_init();
	seeding(k_chal);
	unsigned char * kjc[q];
	for(j=0;j<q;j++) {
		kjc[j] = malloc(16*sizeof(unsigned char *));
		keygen(kjc[j], 16);
	}
    // use kjc to generate random indices
	for(j=0;j<q;j++) {
		keygen_init();
		seeding(kjc[j]);
		for(p=0;p<v;p++) {
			unsigned long randomIndex;
			char rand[8];
			keygen(rand, 8);
			randomIndex = *(unsigned long *)rand;	
			c[j].s[p] = randomIndex % t;
		}
	}
    // use k_ind to generate random index u
	keygen_init();
	seeding(k_ind);	
	for(j=0;j<q;j++) {
		unsigned int randomIndex;
		char rand[4];
		keygen(rand, 4);
		randomIndex = *(unsigned int *)rand;	
		c[j].u = randomIndex % w;
	}
	printf("Precomputation for challenges finishes\n");
    // precompute challenge responses
	precompute_response(fp,c,k_enc);
	printf("Precomputation for responses finishes\n");
    chalTime+=getCPUTime()-chalStartTime - write2Time;
    // append MAC at the end of the files
	printf("\nAppend MAC to the end of the file...\n");
    write2StartTime = getCPUTime();
    clock_gettime(CLOCK_MONOTONIC, &start);
	fwrite(mac,16,1,fp);
    clock_gettime(CLOCK_MONOTONIC, &finish);
    double addTime = finish.tv_sec - start.tv_sec;
    addTime += (finish.tv_nsec - start.tv_nsec)/1000000000.0;
    write2Time += getCPUTime() - write2StartTime+addTime;
    fseek(fp,0,SEEK_END);
	fileLen2 = ftell(fp);
	fclose(fp);

	printf("\nPOR encoding done\n");

	// display time performance
    totalTime = getCPUTime() - totalStartTime;
    printf("#RESULT#\n");
    printf("%lu\n",fileLen1);
    printf("%lu\n",fileLen2);
    printf("%lf\n",totalTime);
    printf("%lf\n",readTime);
    printf("%lf\n",prpTime);
    printf("%lf\n",eccTime);
    printf("%lf\n",macTime);
    printf("%lf\n",chalTime);
    printf("%lf\n",write1Time+write2Time);
    printf("%lf\n",encTime);


}



