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

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100

#define MAXDATASIZE 100


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    clock_t Begin = clock();
    FILE *fp;
    
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    
    char * pch;
    char * filename;
    printf("%s\n",buf);
    pch = strtok(buf,"/");
    while (pch != NULL)
    {
        filename = pch;
        pch = strtok (NULL, "/");
    }
    printf("%s\n",filename);
    
    char command[100];
    sprintf(command,"md5 %s",filename);
    fp = fopen(filename,"w");
    int received = 0;

    while((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))>0) {
        received += numbytes;
        printf("%d\n",received);
        if (fwrite(buf, sizeof(char),numbytes, fp) == -1) {
            return -1;
        }
    }
    fclose(fp);
    
    printf("server: finish receive\n");
    clock_t End = clock();
    float elapMilli, elapSeconds, elapMinutes;
    elapMilli = End/1000;     // milliseconds from Begin to End
    elapSeconds = elapMilli/1000;   // seconds from Begin to End
    elapMinutes = elapSeconds/60;   // minutes from Begin to End
    
    printf ("Milliseconds passed: %.2f\n", elapMilli);
    printf ("Seconds passed: %.2f\n", elapSeconds);
    printf ("Minutes passed: %.2f\n", elapMinutes);
    
    system(command);
    
    close(sockfd);

    return 0;
}
