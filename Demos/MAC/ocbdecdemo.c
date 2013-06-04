#include <tomcrypt.h>
int main(int argc, char *argv[])
{
	int idx, err, res;
	unsigned char key[16], nonce[6], pt[16], ct[16], tag[8];
    unsigned long taglen;
	unsigned long dstlen;
	if (register_cipher(&aes_desc)==-1) {
		printf("error registering AES\n");
		return -1;
	}

    FILE *file;
    unsigned long fileLen;
    file = fopen(argv[1],"r");
    taglen = sizeof(tag);
    while (!feof(file)) {
        size_t counter = fread(ct,sizeof(pt),16,file);
        if ((err = ocb_decrypt_verify_memory(
            find_cipher("aes"),key,16,nonce,ct,counter,pt,tag,taglen,&res))!=CRYPT_OK)
        {
            printf("Error: %s\n",error_to_string(err));
            return -1;
        }
    }
    fclose(file);
    printf("ocbdec complete\n");

    return 0;
}