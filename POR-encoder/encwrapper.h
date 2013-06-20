#include <tomcrypt.h>
int err;
symmetric_key skey;

// initialize the enc wrapper
int enc_init(unsigned char* key)
{
    if ((err = aes_setup(key,sizeof(key),0,&skey)) != CRYPT_OK) {
		printf("aes setup: %s\n", error_to_string(err));
        return err;
	}
    return 0;
}


int encrypt(unsigned char * ct, unsigned char * pt)
{
	
    if ((err = aes_ecb_encrypt(pt,ct,&skey)) != CRYPT_OK) {
		printf("aes encrypt error: %s\n", error_to_string(err));
		return err;
	}
    return 0;
}

int decrypt(unsigned char * ct, unsigned char * pt)
{
	
    if ((err = aes_ecb_decrypt(ct,pt,&skey)) != CRYPT_OK) {
		printf("aes decrypt error: %s\n", error_to_string(err));
		return err;
	}
    return 0;
}
