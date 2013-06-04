#include <tomcrypt.h>
int main(int argc, char *argv[])
{
	int idx, err;
	hmac_state hmac;
	unsigned char key[16], dst[MAXBLOCKSIZE];
	unsigned long dstlen;
	if (register_hash(&sha1_desc)==-1) {
		printf("error registering SHA1\n");
		return -1;
	}
	idx = find_hash("sha1");

	dstlen = sizeof(dst);

	if ((err = hmac_file(idx,argv[1],key,16,dst,&dstlen))!=CRYPT_OK) {
		printf("Error hmac: %s\n",error_to_string(err));
		return -1;
	}
    
    printf("hmac complete\n");
    return 0;
}
