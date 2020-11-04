#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc,char* argv[]){
    int stringLen;
    int second=0;
    if(argc<2){
        printf("you must input agrs!");
        exit(1);
    }
    stringLen = strlen(argv[1]);
    if(stringLen!=0){
        for(int i=0;i<stringLen;i++){
            second = second*10+argv[1][i]-'0';
        }
    }
    else{
        second = 1;
    }
    printf("(nothing happens for a little while)\n");
    sleep(second);
    exit(0);
}

// int main(int argn, char *argv[]){
// 	if(argn != 2){
// 		fprintf(2, "must 1 argument for sleep\n");
// 		exit(1);
// 	}
// 	int sleepNum = atoi(argv[1]);
// 	printf("(nothing happens for a little while)\n");
// 	sleep(sleepNum);
// 	exit(0);
// }