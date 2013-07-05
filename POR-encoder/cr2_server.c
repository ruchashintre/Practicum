#define  MAX_LEN  256
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

# define LEN 16;

typedef struct {
int s[v];
int u;
} Chal;

int q,t,v;
static unsigned char * buf;
const char * file_name = "XYZ";
unsigned char * kjc;

main()
{
	// socket programming - to get challenge from client
	int sockfd, newsockfd ; 
	int clilen;
	struct sockaddr_in	cli_addr, serv_addr;	
	int i;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(4000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);
	printf("Server started.........\n");
	while (1) 
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen) ;
		File * fp = fopen(file_name, "a+");

		fseek(fp,0,SEEK_END); // eof + 0
		long fileLen = ftell(fp); // length of the file in bytes

		// for Qj
		int len_minus_mac = fileLen - 16;
		int start_pos = len_minus_mac - 32*q;

		fsetpos(fp,start_pos); // currentof + i*32

		if (newsockfd < 0) 
		{
			printf("Accept error\n");
			exit(0);
		}
		
		for(i=0; i < 100; i++) buf[i] = '\0';
		strcpy(buf,"Connected to Server");
		send(newsockfd, buf, 100, 0);

		if((buf = malloc(2*sizeof(unsigned int) + LEN))==NULL)
		{ 
			fprintf(stderr, "failed to allocate memory for buf.\n");
		}	

 		//receives challenge from the client
		recv(newsockfd, buf, 2*sizeof(unsigned int) + LEN, 0);
		
		Chal * c = malloc(sizeof(Chal));	
		kjc = malloc(LEN);

		int i=0;
		unsigned int j = (unsigned int)buf[0];
		for(i=4;i<20;i++)
		{
			kjc[i-4] = buf[i];
		}
		i = 20;
		c[j].u = (unsigned int)buf[i];
	
		//compute v pseudo random block indices from 1 to t
			/*
			bufint = malloc(v*sizeof(unsigned long));

			for(i=0;i<v;i++)
			{
				//long -> 4 bytes
				unsigned long len = 4;
				unsigned char * buf;
		
				int err = keygen_init();
				err = seeding(kjc);
				keygen(buf, len);
				bufint[i] = *(unsigned long *)buf;
				bufint[i] = bufint[i] % t;
			}
			c[j].s = bufint;			
			*/
		
		// compute Mj
		char * Mj = precompute_response(fp,c); // **this will have to compute Mj for the jth index only, coz it is in j loop**

		// compute Qj from the file
		unsigned char Qj[32];
		fread(Qj, 32, 1, fp); 
		typedef unsigned long fpos_t;
		fpos_t * pos;
		fgetpos(fp,pos);
			
		fsetpos(fp,pos + 32);

		// compute response
		char * response = malloc(64);
		int l=0;
		for(m=0;m<32;m++)
		{
			response[l] = Mj[m];
			l++;
		}
		for(m=0;m<32;m++)
		{
			response[l] = Qj[m]; 
			l++;
		}

		// send response
		send(newsockfd, response, 64, 0);
		free(buf);		

		close(newsockfd);
	}
}

