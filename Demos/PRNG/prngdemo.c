#include <tomcrypt.h>

int main(int argc, char *argv[])
{
	printf("----------prng demo---------\n");
	prng_state prng;
	unsigned char buf[100];
	int err;
	printf("prng start...\n");
	if ((err = yarrow_start(&prng)) != CRYPT_OK) {
		printf("start error: %s\n", error_to_string(err));
	}
	printf("prng seeding...\n");
	if ((err = yarrow_add_entropy(argv[1], strlen(argv[1]), &prng))!= CRYPT_OK) {
		printf("Add entropy error: %s\n", error_to_string(err));
	}
	printf("prng ready...\n");
	if ((err = yarrow_ready(&prng)) != CRYPT_OK) {
		printf("Ready error: %s\n", error_to_string(err));
	}
	printf("Read %lu bytes from prng\n",yarrow_read(buf,sizeof(buf),&prng));
	printf("prng info:%u",buf[0]);
	int i = 0;
	for(i=1;i<100;i++)
	{
		printf(" %u",buf[i]);
	}
	printf("\n");	
}
