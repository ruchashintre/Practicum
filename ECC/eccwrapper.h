#include <stdio.h>
#include <stdlib.h>
#include "ecc.h"
#include <string.h>

unsigned char *codeword;
FILE *file;
unsigned long fileLen;
unsigned char *buffer;
int n,k,d;
int erasures[16];
int nerasures = 0;

/* Initialize the reed-solomon code parameters
    np  length of codeword
    Kp  length of msg
    dp  distance
 */
int
initialize (int np, int kp, int dp)
{
    if (np>256) {
        fprintf(stderr, "codeword size exceed 256");
        return -1;
    }
    if (kp+dp != np) {
        fprintf(stderr, "Wrong parameters! n != k + d");
        return -1;
    }
    if (np<=0||kp<=0||dp<=0) {
        fprintf(stderr, "Wrong parameters! Must be positive");
        return -1;
    }
    initialize_ecc ();
    n = np;
    k = kp;
    d = dp;
    codeword = (unsigned char *)malloc(n);
    if (!codeword)
    {
        fprintf(stderr, "Memory error!");
        return -1;
    }
    return 0;
}

int
rsencode (char *filename)
{        
    //Open file
    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        return -1;
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
        return -1;
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    
    fclose(file);
    
    long i = fileLen;
    long pos = 0;
    int writeFlag = TRUE;
    strcat(filename,".encode");
    printf("encoding the original file into %s\n",filename);
    printf("file size %ld\n",fileLen);
    
    while (i>0)
    {
        int msgLen;
        // read k byte message
        if (i>k) {
            msgLen = k;
        }
        else {
            msgLen = i;
        }
        
        unsigned char msg[msgLen];
        int j;
        
        for(j=0;j<msgLen;j++) {
            msg[j] = buffer[pos];
            pos++;
        }
        // encode message
        encode_data(msg, sizeof(msg), codeword);
        
        // write to encode file
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
    return 0;
}

int
rsdecode (char *filename)
{
    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", filename);
        return -1;
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
        return -1;
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    
    fclose(file);
    int i = fileLen;
    long pos = 0;
    int writeFlag = TRUE;
    strcat(filename,".recover");
    printf("recovering corrupted file into %s\n",filename);
    printf("file size %ld\n",fileLen);

    while (i>0)
    {
        int msgLen;
        // read codeword with length n
        if (i>n) {
            msgLen = n;
        }
        else {
            msgLen = i;
        }
        unsigned char msg[msgLen];
        int j;
        
        for(j=0;j<msgLen;j++) {
            msg[j] = buffer[pos];
            pos++;
        }
        // decode codeword into message
        decode_data(msg, msgLen);
        
        if (check_syndrome () != 0) {
            // correct errors
            correct_errors_erasures (msg,msgLen,nerasures,erasures);
        }
        if (writeFlag) {
            file = fopen(filename, "w+");
            writeFlag = FALSE;
        }
        else {
            file = fopen(filename, "a+");
        }
        // write recovered message into file
        fwrite(msg, msgLen-d, 1, file);
        
        fclose(file);
        i = i - msgLen;
    }
    
    free(buffer);
    return 0;
}