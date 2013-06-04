#include <tomcrypt.h>
#include "jg_timing.h" /*Perform timing measurements*/

void quicksort(int list1[], unsigned int list2[], int m, int n);

int main(int argc, char *argv[])
{
    int i;
    double startTime, endTime;
    startTime = getCPUTime();
//    char * filename = argv[2];
    //printf("Original Message: %s\n",input);
	prng_state prng;
//    FILE *file;
//    FILE *prpfile;
//    file = fopen(filename,"r");
//    prpfile = fopen("prpfile","w+");
//    if (!file)
//    {
//        fprintf(stderr, "Unable to open file %s", filename);
//        return -1;
//    }
    
//    fseek( file, 0, SEEK_END);
//    unsigned long fileLen;
//    fileLen = ftell(file);
//    printf("file size: %lu\n",fileLen);
//    int blocks = fileLen / 256 +1;
    int filesize = atoi(argv[2]);
    printf("file size: %dG\n",filesize);
    int blocks = filesize * (1<<25);
    printf("block num: %d\n",blocks);
    //int blocks = strlen(input);
	unsigned char buf[4];
    int * prptable;
//    int * invprptable;
    unsigned int * bufint;
    bufint = malloc(blocks*sizeof(unsigned int));
//    buf = (unsigned char *)malloc(4*sizeof(unsigned char));
    prptable = malloc(blocks*sizeof(int));
//    invprptable = (int *)malloc(blocks*sizeof(int));

    for (i=0;i<blocks;i++) {
//        printf("%d\n",i);
//        fflush(stdout);
        prptable[i] = i;
    }

	int err;
	if ((err = yarrow_start(&prng)) != CRYPT_OK) {
		printf("start error: %s\n", error_to_string(err));
	}
	if ((err = yarrow_add_entropy(argv[1], strlen(argv[1]), &prng))!= CRYPT_OK) {
		printf("Add entropy error: %s\n", error_to_string(err));
	}
	if ((err = yarrow_ready(&prng)) != CRYPT_OK) {
		printf("Ready error: %s\n", error_to_string(err));
	}
    for (i=0;i<blocks;i++) {
        yarrow_read(buf,4,&prng);
        bufint[i] = *(unsigned int *)buf;
//        printf("%d:%d\n",i,bufint[i]);
    }



//	printf("prng info:%u",buf[0]);
    
//	for(i=1;i<blocks;i++)
//	{
//		printf(" %u",buf[i]);
//	}
//    printf("\n");
    printf("start sorting\n");

    quicksort(prptable,bufint,0,blocks-1);
    printf("finish sorting\n");

    //printf("sorted num:%u",buf[0]);
    
//	for(i=1;i<blocks;i++)
//	{
//		printf(" %u",buf[i]);
//	}
//    printf("\n");
    
    //printf("Permutation table:\n");
    
//	for(i=0;i<blocks;i++)
//	{
//		//printf("%d -> %d\n",i,prptable[i]);
//        invprptable[prptable[i]] = i;
//	}
    printf("done prp\n");
    fflush(stdout);
//    for(i=0;i<blocks;i++)
//	{
//		int blockind = invprptable[i];
//        fseek(file,blockind*256,SEEK_SET);
//        unsigned char buffer[256];
//        fread(buffer, 256, 1, file);
//        fwrite(buffer, 256, 1, prpfile);
//	}
//    fclose(file);
//    fclose(prpfile);
    printf("done file permutation\n");
    endTime = getCPUTime();
    fprintf( stderr, "CPU time used = %lf\n", (endTime - startTime) );
}

void swapint (int *x, int *y) {
    int temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

void swapchar (unsigned char *x, unsigned char *y) {
    unsigned char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

int pivot(int i, int j) {
    return (i+j)/2;
}

void quicksort(int list1[], unsigned int list2[], int m, int n) {
    int key;
    int i, j, k;
    
    if (m<n) {
        k = pivot(m,n);
        swapint(&list1[m],&list1[k]);
        swapint(&list2[m],&list2[k]);
        key = list2[m];
        i = m+1;
        j = n;
        while (i<=j) {
            while ((i<=n)&&(list2[i]<=key))
                i++;
            while ((j>=m)&&(list2[j]>key))
                j--;
            if (i<j) {
                swapint(&list1[i],&list1[j]);
                swapint(&list2[i],&list2[j]);
            }
        }
        swapint(&list1[m],&list1[j]);
        swapint(&list2[m],&list2[j]);
        quicksort(list1,list2,m,j-1);
        quicksort(list1,list2,j+1,n);
    }
}
