#include <tomcrypt.h>
int err;
symmetric_key skey;

// initialize the enc wrapper
int enc_init(unsigned char* key)
{
	int keysize = 16;
	unsigned char keycopy[keysize];
	int i;
	for (i=0;i<keysize;i++) {
		keycopy[i] = key[i];
	}
    if ((err = aes_setup(key,sizeof(keycopy),0,&skey)) != CRYPT_OK) {
		printf("aes setup: %s\n", error_to_string(err));
		    	aes_keysize(&keysize);
		printf("aes suggested keysize: %d\n", keysize);
		printf("size of current key: %lu\n", sizeof(key));
        return err;
	}
    return 0;
}


int encrypt(unsigned char* ct, unsigned char* pt,int len)
{
	int i;
	unsigned char copyct[len],copypt[len];
	for (i=0;i<len;i++) {
		copypt[i] = pt[i];
	}
    if ((err = aes_ecb_encrypt(copypt,copyct,&skey)) != CRYPT_OK) {

		printf("aes encrypt error: %s\n", error_to_string(err));
		return err;
	}
	for (i=0;i<len;i++) {
		ct[i] = copyct[i];
	}
    return 0;
}

int decrypt(unsigned char * ct, unsigned char * pt,int len)
{
	int i;
	unsigned char copyct[len],copypt[len];
	for (i=0;i<len;i++) {
		copyct[i] = ct[i];
	}	
    if ((err = aes_ecb_decrypt(copyct,copypt,&skey)) != CRYPT_OK) {
		printf("aes decrypt error: %s\n", error_to_string(err));
		return err;
	}
	for (i=0;i<len;i++) {
		pt[i] = copypt[i];
	}
    return 0;
}