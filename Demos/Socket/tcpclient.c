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

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

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
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    FILE *fp;
    int fd;

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname filename\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

 // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
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

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
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
    
    if (send(sockfd, argv[2], MAXDATASIZE-1, 0) == -1)
        perror("send");
    
    int sended=0;
    //while ((nbytes = fread(file_data,sizeof(char),MAXDATASIZE-1,fp))>0)
    while ((nbytes = read(fd,file_data,MAXDATASIZE-1))>0)
    {
        sended+=nbytes;
        printf("%d / %d\n",sended,sz);
        int sendbytes;
//        printf("%s\n",file_data);
        if ((sendbytes = send(sockfd, file_data, nbytes, 0)) != nbytes)
        {
            printf("nbytes=%d\n",sendbytes);
            printf("sendbytes=%d\n",nbytes);
        }
    }
    fclose(fp);
    close(fd);
    //send(sockfd,"finish",10,0);
    
//    if (sendfile(fd,sockfd,0,len,NULL,0) == -1) {
//        perror("sendfile");
//        exit(1);
//    }
//    close(fd);
//    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
//        perror("recv");
//        exit(1);
//    }

//    buf[numbytes] = '\0';

    printf("client: send finish\n");
    clock_t End = clock();
    float elapMilli, elapSeconds, elapMinutes;
    elapMilli = End/1000;     // milliseconds from Begin to End
    elapSeconds = elapMilli/1000;   // seconds from Begin to End
    elapMinutes = elapSeconds/60;   // minutes from Begin to End
    
    printf ("Milliseconds passed: %.2f\n", elapMilli);
    printf ("Seconds passed: %.2f\n", elapSeconds);
    printf ("Minutes passed: %.2f\n", elapMinutes);
    
    //system(command);

    close(sockfd);

    return 0;
}
