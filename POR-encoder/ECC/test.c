#include <stdio.h>
#include <stdlib.h>
#include "eccwrapper.h"
#include <string.h>
#include <time.h>

#define bool int
#define true 1
#define false 0
int
main (int args, char *argv[])
{
    initialize(255,223,32);
    time_t start,end;
    double dif;
    time(&start);
    printf("Start encode time: %s",ctime(&start));
    rsencode(argv[1]);
    time(&end);
    printf("Finish encode time: %s\n",ctime(&end));
    dif = difftime(end,start);
    printf("Encode time taken: %f\n",dif);
    printf("=================================\n");

    time(&start);
    printf("Start decode time: %s",ctime(&start));
    rsdecode(argv[1]);
    time(&end);
    printf("Finish decode time: %s\n",ctime(&end));
    dif = difftime(end,start);
    printf("Decode time taken: %f\n",dif);
}