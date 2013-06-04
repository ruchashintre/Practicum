#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>

#define SERVERPORT "4950"    // the port users will be connecting to

#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    FILE *fp;
    int fd;

    if (argc != 3) {
        fprintf(stderr,"usage: tcpclient hostname filename\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to bind socket\n");
        return 2;
    }

    char command[100];
    sprintf(command,"md5 %s",argv[2]);
    //system(command);
    fp = fopen(argv[2], "r");
    clock_t Begin = clock();
    
    char file_data[MAXDATASIZE];
    
    size_t nbytes = 0;
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    printf("%d\n",sz);
    
    fd=open(argv[2], O_RDONLY);
    
    if (sendto(sockfd, argv[2], MAXDATASIZE-1, 0,p->ai_addr, p->ai_addrlen) == -1)
        perror("send");
    
    int sended=0;
    //while ((nbytes = fread(file_data,sizeof(char),MAXDATASIZE-1,fp))>0)
    while ((nbytes = read(fd,file_data,MAXDATASIZE-1))>0)
    {
        sended+=nbytes;
        printf("%d / %d\n",sended,sz);
        int sendbytes;
        //        printf("%s\n",file_data);
        if ((sendbytes = sendto(sockfd, file_data, nbytes, 0,p->ai_addr, p->ai_addrlen)) != nbytes)
        {
            printf("nbytes=%d\n",sendbytes);
            printf("sendbytes=%d\n",nbytes);
        }
    }
    fclose(fp);
    close(fd);
    
    sendto(sockfd, NULL, 0, 0,p->ai_addr, p->ai_addrlen);
    
    printf("client: send finish\n");
    clock_t End = clock();
    float elapMilli, elapSeconds, elapMinutes;
    elapMilli = End/1000;     // milliseconds from Begin to End
    elapSeconds = elapMilli/1000;   // seconds from Begin to End
    elapMinutes = elapSeconds/60;   // minutes from Begin to End
    
    printf ("Milliseconds passed: %.2f\n", elapMilli);
    printf ("Seconds passed: %.2f\n", elapSeconds);
    printf ("Minutes passed: %.2f\n", elapMinutes);
    
    system(command);
    //if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
    //         p->ai_addr, p->ai_addrlen)) == -1) {
    //    perror("client: sendto");
    //    exit(1);
    //}

    freeaddrinfo(servinfo);

    //printf("client: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);

    return 0;
}
