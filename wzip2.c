#include<stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc,char *argv[]) {
   if (argc == 1){
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
   }

    FILE *f;
    int previous = 0;
    int charcount = 0;
    int i = 1;
    while(argv[i] != NULL){

        f = fopen(argv[i],"r");
        if(f == NULL){
            printf("cant open file\n");
            exit(1);
        }

        int cr;
        while ((cr =fgetc(f)) != -1){
           //compare the current char with the last read char
           if(cr != previous && previous != 0){
            fwrite(&charcount, sizeof(int) , 1,
                     stdout);
            fwrite(&previous, sizeof(char) , 1,
                     stdout);
            charcount = 1;
           }else{
           charcount++;
           }
           previous = cr;
        }         
   i++;
    }
 fwrite(&charcount, sizeof(int) ,1,
                     stdout);
 fwrite(&previous, sizeof(char) , 1,
                     stdout);
return 0;
}
       