//Demo for AONT Functionality : Reference : AONT and the Package Transform by Rivest L. Ronald

#include <tomcrypt.h>

int main(int argc, char *argv[])
{
	int i;
	char * filename = argv[2];
	prng_state prng;
	FILE *file;
	FILE *prpfile;
	unsigned long fileLen;
	char * key;
	symmetric_key * skey;
	char * key1;
	int err,errno;

	key="pooja";
	key1 = "poojaulhasdeesai1";
	
	file = fopen(filename,"r");
	prpfile = fopen("prpfile","w+");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", filename);
		return -1;
	}

	//go to the end of the file
	fseek( file, 0, SEEK_END);

	//find file length
	fileLen = ftell(file);

	//divide file into blocks of 128 bits
	int blocks = fileLen / 16 +1;
    	unsigned char buf[blocks];

	for(i=1;i<blocks;i++)
	{
		printf(" %u",buf[i]);
	}
   	printf("\n");
    
	/*Hard coded 40 bit encryption key*/
	printf("This is the 40 bit key for outer package transform:%s\n", key);

	/* you must register a cipher before you use it */
	if (register_cipher(&rc5_desc) == -1) {
		printf("Unable to register RC5 cipher.");
		return -1;
	}
	
	/* generic call to function (assuming the key
	* in key[] was already setup) */
	if ((err =cipher_descriptor[find_cipher("rc5")].setup(key, 8, 0, skey)) != CRYPT_OK) {
		printf("Error setting up rc5: %s\n", error_to_string(err));
		return -1;
	}
	
	/*Do the outer package transform
	Here, mi' =  mi EXOR E(k,i)*/
	
	unsigned char encrypt_op[16];
    	for(i=0;i<blocks;i++)
	{
		//convert i to string -> encrypt_op should be a string
		char * str;
		sprintf(str, "%d", i);
		str = PadLeft(str,16,"0");		

	  	if ((errno = rc5_ecb_encrypt(str,encrypt_op,skey)) != CRYPT_OK) {
           	 	printf("encrypt error: %s\n", error_to_string(errno));
           		exit(-1);
         	}
        	
		fseek(file,i*16,SEEK_SET);
        	unsigned char buffer[16];
		unsigned char transformed_buffer[16];
    
		/*read the file block*/
		fread(buffer, 16, 1, file);
		
		/*do bitwise xor with E(k',i)*/
		transformed_buffer[i] = stringXor(buffer,encrypt_op);
		
        	fwrite(transformed_buffer, 16, 1, prpfile);
	}
	
	int j;
	
	if ((err =cipher_descriptor[find_cipher("rc5")].setup(key1, 8, 0, skey)) != CRYPT_OK) {
		printf("Error setting up rc5: %s\n", error_to_string(err));
		return -1;
	}
	printf("This is the 128 bit publicly known key:%s", key1);
	
       //set  up rc5 again  as we pass symmetric key to it.  	
	unsigned char *last_block;
	unsigned char *h[blocks];
	for (j=0;j<blocks;j++){
		
		char *str;
		sprintf(str, "%d", i);
		str = PadLeft(str,16,"0");		

		unsigned char buffer[16];
		
		/*read the file block*/
		fread(buffer, 16, 1, prpfile);
		
		/*hi = E(K,mi' xor i)*/
		char *temp = XorString(buffer,str);
		
		if ((errno = rc5_ecb_encrypt(temp,h[j],skey)) != CRYPT_OK) {
           	 	printf("encrypt error: %s\n", error_to_string(errno));
           		exit(-1);
         	}
     	}

	char str[16];
	sprintf(str, "%d",0);
	h[0] = PadLeft(str,16,"0");

	sprintf(str, "%d", 1);
	h[1] = PadLeft(str,16,"0");

	last_block = XorString(h[0],h[1]);
	for(j=2;j<blocks;j++){	
		last_block = XorString(last_block,PadLeft(h[j],16,"0"));
	}

	last_block =  XorString(last_block,PadLeft(key,16,"0"));
	fwrite(last_block, 16, 1, prpfile);
    
	fclose(file);
	
	fclose(prpfile);   
}

char * PadLeft(char *string, int padded_len, char *pad) {
    int len = (int) strlen(string);
    if (len >= padded_len) {
        return string;
    }
    int i;
    for (i = 0; i < padded_len - len; i++) {
        strcat(pad,string);
    }
    return pad;
}

char * XorString(char *str1, char *str2){
{
  char * output;
  for(int x=0; x<strlen(str1); x++){
    output[x]=str1[x]^str2[x];
  }
  return output;
}

