#include <tomcrypt.h>
prng_state prng;
int err;

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
int seeding(char * seed)
{
    if ((err = fortuna_add_entropy(seed, strlen(seed), &prng))!= CRYPT_OK) {
		printf("Add seed error: %s\n", error_to_string(err));
        return err;
	}
    return 0;
}

// generate key
int keygen(unsigned char * buf, unsigned long len)
{
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
    fortuna_read(buf,len,&prng);
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
