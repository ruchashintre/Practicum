#include <tomcrypt.h>
int main(int argc, char *argv[])
{
	int idx, err;
	omac_state omac;
	unsigned char key[16], dst[MAXBLOCKSIZE];
	unsigned long dstlen;
	if (register_cipher(&aes_desc)==-1) {
		printf("error registering AES\n");
		return -1;
	}

	idx = find_cipher("aes");

	dstlen = sizeof(dst);

	if ((err = omac_file(idx,key,16,argv[1],dst,&dstlen))!=CRYPT_OK) {
		printf("Error hmac: %s\n",error_to_string(err));
		return -1;
	}

    printf("omac complete\n");
    return 0;
}
