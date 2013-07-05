#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include "keygenwrapper.h"
#include "encwrapper.h"

#define PORT "3490" // the port client will be connecting to 
#define v 1024/32
#define w 4096/32
#define LEN 16
#define BLOCK_SIZE 32
#define q 100
#define MAXDATASIZE 100 // max number of bytes we can get at once 

extern unsigned char k_file_perm[16],k_ecc_perm[16],k_ecc_enc[16],
	k_chal[16],k_ind[16],k_enc[16],k_mac[16];


void displayCharArray(unsigned char* out,int len)
{
	int i;
	for (i = 0;i < len; i++) {
		printf("%02x", out[i]);
	}
	printf("\n");
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	//char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	FILE *fp;
	int fd;

	if (argc != 3) {
		fprintf(stderr,"usage: ./client hostname masterkey\n");
		exit(1);
	}
    
	master_keygen(argv[2]);
	unsigned char buf[24];
	int j;
	keygen_init();
	seeding(k_chal);
	unsigned char * kjc[q];
	for(j=0;j<q;j++) {
		kjc[j] = malloc(16*sizeof(unsigned char *));
		keygen(kjc[j], 16);
	}
	keygen_init();
	seeding(k_ind);	
	unsigned int u[q];
	for(j=0;j<q;j++) {
		unsigned int randomIndex;
		char rand[4];
		keygen(rand, 4);
		randomIndex = *(unsigned int *)rand;	
		u[j] = randomIndex % w;
	}
	for (j=0;j<q;j++) {
		buf[0] = (j >> 24) & 0xFF;
		buf[1] = (j >> 16) & 0xFF;
		buf[2] = (j >> 8) & 0xFF;
		buf[3] = j & 0xFF;
		int i;
		for (i=0;i<16;i++)
			buf[4+i] = kjc[j][i];
		buf[20] = (u[j] >> 24) & 0xFF;
		buf[21] = (u[j] >> 16) & 0xFF;
		buf[22] = (u[j] >> 8) & 0xFF;
		buf[23] = (u[j]) & 0xFF;
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

	 // loop through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("client: connect");
				continue;
			}
			break;
		}

		if (p == NULL) {
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
		printf("client: connecting to %s\n", s);

		freeaddrinfo(servinfo); // all done with this structure
    	if (send(sockfd, buf, 24, 0) == -1)
        perror("send");
    
    	printf("client: send challenge #%d\n",j);
		printf("j=%d\nkjc=",j);
		displayCharArray(kjc[j],16);
		printf("u=%d\n",u[j]);
		displayCharArray(buf,24);
		unsigned char recvbuf[64];
		if ((numbytes = recv(sockfd, recvbuf, 32, 0)) == -1) {
			perror("recv");
		}
    	printf("client: receive response M%d\n",j);
		displayCharArray(recvbuf,32);
		if ((numbytes = recv(sockfd, recvbuf, 32, 0)) == -1) {
			perror("recv");
		}
    	printf("client: receive response Q%d\n",j);
		displayCharArray(recvbuf,32);
		printf("client: Decrypting Q%d, dec(Q%d)=\n",j,j);
		enc_init(k_enc);
		unsigned char newMj[32];
		decrypt(recvbuf, newMj,32);
		displayCharArray(newMj,32);
	}

    
    //system(command);

    close(sockfd);

    return 0;
}
