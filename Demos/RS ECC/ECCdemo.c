#include <stdio.h>
#include <stdlib.h>
#include "ecc.h"
#include <string.h>

//unsigned char msg[] = "Nervously I loaded the twin ducks aboard the revolving platform.";

unsigned char codeword[256];
FILE *file;
unsigned long fileLen;
unsigned char *buffer;

/* Some debugging routines to introduce errors or erasures
 into a codeword.
 */

/* Introduce a byte error at LOC */
void
byte_err (int err, int loc, unsigned char *dst)
{
    printf("Adding Error at loc %d, data %#x\n", loc, dst[loc-1]);
    dst[loc-1] ^= err;
}

/* Pass in location of error (first byte position is
 labeled starting at 1, not 0), and the codeword.
 */
void
byte_erasure (int loc, unsigned char dst[], int cwsize, int erasures[])
{
    printf("Erasure at loc %d, data %#x\n", loc, dst[loc-1]);
    dst[loc-1] = 0;
}


int
main (int argc, char *argv[])
{
    int erasures[16];
    int nerasures = 0;
    char* filename = argv[1];
    char command[100];
    printf("reading the original file %s\n",argv[1]);

    //filename = argv[1];
    sprintf(command,"md5 %s",argv[1]);
    system(command);

    /* Initialization the ECC library */
    
    initialize_ecc ();
    
    //Open file
    file = fopen(argv[1], "r");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", argv[1]);
        exit(1);
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);
    
    //Allocate memory
    buffer=(unsigned char *)malloc(fileLen+1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(2);
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);

    fclose(file);

    long i = fileLen;
    long pos = 0;
    int writeFlag = TRUE;
    strcat(filename,".encode");
    printf("----------------------------------------------\n");
    printf("encoding the original file into %s\n",filename);

    while (i>0)
    {
        //system(command);

        int msgLen;
        if (i>256-NPAR) {
            msgLen = 256-NPAR;
        }
        else {
            msgLen = i;
        }
        unsigned char msg[msgLen];
        int k;

        for(k=0;k<msgLen;k++) {
            msg[k] = buffer[pos];
            pos++;
        }
        encode_data(msg, sizeof(msg), codeword);

        if (writeFlag) {
            file = fopen(filename, "w+");
            writeFlag = FALSE;
        }
        else {
            file = fopen(filename, "a+");
        }
        fwrite(codeword, msgLen+NPAR, 1, file);

        fclose(file);
        i = i - msgLen;
    }
    free(buffer);

    sprintf(command,"md5 %s",filename);
    system(command);

    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        exit(1);
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);
    //Allocate memory
    buffer=(unsigned char *)malloc(fileLen+1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(2);
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    
    fclose(file);
    i = fileLen;
    pos = 0;
    writeFlag = TRUE;
    strcat(filename,".corrupt");
    printf("----------------------------------------------\n");
    printf("making some errors and write into the file %s\n",filename);

    int k;
    for(k = 0;k<10;k++) {
        long h = rand()%fileLen;
        buffer[h] = buffer[h] % 20;
    }
    file = fopen(filename, "w+");
    fwrite(buffer, fileLen, 1, file);
    fclose(file);

    free(buffer);

    sprintf(command,"md5 %s",filename);
    system(command);
    
    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        exit(1);
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);
    //Allocate memory
    buffer=(unsigned char *)malloc(fileLen+1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(2);
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    
    fclose(file);
    i = fileLen;
    pos = 0;
    writeFlag = TRUE;
    strcat(filename,".recover");
    printf("----------------------------------------------\n");
    printf("recovering corrupted file into %s\n",filename);
    while (i>0)
    {
        int msgLen;
        if (i>256) {
            msgLen = 256;
        }
        else {
            msgLen = i;
        }
        unsigned char msg[msgLen];
        int k;
        
        for(k=0;k<msgLen;k++) {
            msg[k] = buffer[pos];
            pos++;
        }
        decode_data(msg, msgLen);

        if (check_syndrome () != 0) {
            correct_errors_erasures (msg,msgLen,nerasures,erasures);
        }
        if (writeFlag) {
            file = fopen(filename, "w+");
            writeFlag = FALSE;
        }
        else {
            file = fopen(filename, "a+");
        }
        fwrite(msg, msgLen-NPAR, 1, file);
        
        fclose(file);
        i = i - msgLen;
    }
    sprintf(command,"md5 %s",filename);
    system(command);
    
    free(buffer);
    exit(0);
}

