#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "por.h"

#define PORT "3490"  // the port users will be connecting to
#define LEN 16
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 100

static unsigned long t;
static unsigned	char uth[BLOCK_SIZE];

// concatenate two RS code for encoding
void concat_encode(unsigned char * message,unsigned char* codeword) {
	unsigned char tmp_code[v*BLOCK_SIZE*n1/k1],stripe[k1],stripe_code[n1];
	int index,i,j;
	// read in the message part
	for (index=0;index<v*BLOCK_SIZE;index++) {
		tmp_code[index] = message[index];
	}
	// outer encoding on the message
	for (i=0;i<v;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = message[i*k1+j];
		}
		initialize_ecc();
		// encode for each outer stripe
		encode_data(stripe,k1,stripe_code);
		for (j=0;j<n1-k1;j++) {
			// append parity at the end
			tmp_code[index] = stripe_code[k1+j];
			index++;
		}
	}
	// perform inner encoding
	index = 0;
	for (i=0;i<v*n1/k1;i++) {
		for (j=0;j<sizeof(stripe);j++) {
			stripe[j] = tmp_code[i*k2+j];
		}
		// encode for each inner stripe
		encode_data(stripe,k2,stripe_code);
		for (j=0;j<n2;j++) {
			codeword[index] = stripe_code[j];
			index++;
		}
	}
}

// execute challenge and generate the response
unsigned char * execute_challenge(FILE* fp, int j, char * kjc, int u, unsigned long * indices) {
	unsigned char message[v*BLOCK_SIZE];
	unsigned char codeword[w*BLOCK_SIZE];

	int pos = 0;
	int i,p;
	keygen_init();
	// using kjc for keygen
	seeding(kjc);
	// generate random indices from kjc
	for(p=0;p<v;p++) {
		unsigned long randomIndex;
		char rand[8];
		keygen(rand, 8);
		indices[p] = *(unsigned long *)rand%t;
	}
	// retrieve the corresponding blocks from random indices
	int index = 0;
	for (i=0;i<v;i++) {
		fseek(fp,indices[i]*BLOCK_SIZE,SEEK_SET);
		unsigned char buffer[BLOCK_SIZE];
		fread(buffer, BLOCK_SIZE, 1, fp);
		for(p=0;p<BLOCK_SIZE;p++) {
			message[index] = buffer[p];
			index++;
		}
	}
	// perform concatenated encoding on the message
	concat_encode(message,codeword);
	// get the u-th symbol of the code
	for (i=0;i<BLOCK_SIZE;i++) {
		uth[i] = codeword[BLOCK_SIZE*u+i];
	}
	return uth;
}

// handle child's signal for fork
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char ** argv)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int numbytes;
    //char buf[MAXDATASIZE];
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int fd;
    //t= atoi(argv[2]);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    	}

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        	}

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        	}

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        	}

        break;
    	}

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    	}

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		// accept the connection
		if (new_fd == -1) {
		    perror("accept");
		    continue;
		}

		inet_ntop(their_addr.ss_family,
		    get_in_addr((struct sockaddr *)&their_addr),
		    s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			unsigned char buf[24];
			// receive message from new_fd
			if ((numbytes = recv(new_fd, buf, 24, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			printf("server: received %d bytes\n",numbytes);
			displayCharArray(buf,24);
			int j;
			unsigned char kjc[16];
			unsigned int u;
			// first 4 bytes are j
			j = buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3];
			// next 16 bytes are kjc
			int i;
			for (i=0;i<16;i++)
				kjc[i] = buf[4+i];
			// last 4 bytes are u
			u = buf[20]<<24 | buf[21]<<16 | buf[22]<<8 | buf[23];
	    	printf("server: receive challenge #%d\n",j);
			printf("j=%d\nkjc=",j);
			displayCharArray(kjc,16);
			printf("u=%d\n",u);
			unsigned long indices[v];
			// read the file for computing response
			FILE *fp;
			unsigned long fileLen;
			if ((fp = fopen(argv[1], "r")) == NULL){
				printf("couldn't open file for reading.\n");
				return -1;
			}
			fseek(fp,0,SEEK_END);
			fileLen = ftell(fp);
			// get the number of blocks
			t = (fileLen - 16 - q*BLOCK_SIZE) / BLOCK_SIZE;
			printf("t=%lu\n",t);
			// execute the challenge to get response
			unsigned char * code = execute_challenge(fp,j,kjc,u,indices);
			// send computed response
			if (send(new_fd, code, BLOCK_SIZE, 0) == -1)
        		perror("send");
        	printf("server: send response M%d\n",j);
        	displayCharArray(code,BLOCK_SIZE);
        	// get and send appended response
        	fseek(fp,t*BLOCK_SIZE+j*BLOCK_SIZE,SEEK_SET);
        	fread(code,BLOCK_SIZE,1,fp);
			if (send(new_fd, code, BLOCK_SIZE, 0) == -1)
        		perror("send");
        	printf("server: send response Q%d\n",j);
        	displayCharArray(code,BLOCK_SIZE);
        	fclose(fp);
			close(new_fd);
			exit(0);
        	}
        
		close(new_fd);  // parent doesn't need this
	}

    return 0;
}
