#include <tomcrypt.h>
prng_state prng;
int err;

unsigned char k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];

// initialize the keygen wrapper
int keygen_init()
{
    if ((err = fortuna_start(&prng)) != CRYPT_OK) {
		printf("start error: %s\n", error_to_string(err));
        return err;
	}
    return 0;
}

// add seed
int seeding(unsigned char * seed)
{
	//printf("print seed %s %\n",seed,sizeof(seed));
    if ((err = fortuna_add_entropy(seed, sizeof(seed), &prng))!= CRYPT_OK) {
		printf("Add seed error: %s\n", error_to_string(err));
        return err;
	}
    return 0;
}

int master_keygen(unsigned char * seed)
{
	keygen_init();
	seeding(seed);
	keygen(k_file_perm,16);
	keygen(k_ecc_perm,16);
	keygen(k_ecc_enc,16);
	keygen(k_chal,16);
	keygen(k_ind,16);
	keygen(k_enc,16);
	keygen(k_mac,16);
}

// generate key
int keygen(unsigned char * buf, unsigned long len)
{
	//memset(buf,0,len * sizeof(*buf));
    /*FILE *key;
    key = fopen(strcat(keyname,".key"),"w");
    if (!key)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        return -1;
    }*/
    if ((err = fortuna_ready(&prng)) != CRYPT_OK) {
		printf("Ready error: %s\n", error_to_string(err));
	}
	//printf("fortuna read %lu %lu\n",sizeof(buf),len);
    int i;
    for(i=0;i<len;i++) {
    	fortuna_read(buf++,1,&prng);
    }
    //fwrite(buf, len, 1, key);
    return 0;
}
/*
// get key
char * getkey(char * keyname)
{
    FILE * key;
    char * buf;
    key = fopen(strcat(keyname,".key"),"r");
    if (!key)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        return NULL;
    }
    fseek(key, 0, SEEK_END);
    int fileLen=ftell(file);
    fseek(key, 0, SEEK_SET);
    buf = malloc (fileLen+1);
    fread(buf, fileLen, 1, key);
    return buf;
}
*/
