#include <sys/types.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
#include "user.h"
// int* selectPrime(int cur_num){
//     return nullptr;
// }

void sendOriginalData(int k){
    int i;
    int n;
    for(i=0;i<34;i++){
        n = i+2;
        write(k,&n,sizeof(i));
    }
}

void sendNewData(int cur_num,int w,int r){
    int i;
    while(read(r,&i,sizeof(i))){
        if((i%cur_num)!=0){
            write(w,&i,sizeof(i));
        }
    }
}

void redirect(int k, int pd[]) {
  close(k);
  dup(pd[k]);
  close(pd[0]);
  close(pd[1]);
}

void childproccess(int r){
    int pid;
    int cur_num;
    int pd[2];
    int len;
    len = read(r,&cur_num,sizeof(cur_num));
    
    if(len!=0){
        if(pipe(pd)<0){
            printf("parent pipe error!/n");
            exit();
        }

        if((pid =fork())<0){
            printf("fork error!,%d,%d",getpid(),pid);
            exit();
        }
        
        if(pid==0){
            // redirect(0,pd);
            close(pd[1]);
            childproccess(pd[0]);
            exit();
        }else{
            printf("prime %d\n",cur_num);
            // redirect(1,pd);
            close(pd[0]);
            sendNewData(cur_num,pd[1],r);
            exit();
        }
    }
    
    exit();

}

int main(int argc,char* argv[]){
    int pd[2];
    int pid;
    if(argc>=2){
        printf("too much argvs!");
        exit();
    }
    if(pipe(pd)<0){
        printf("pipe error!/n");
        exit();
    }

    if((pid =fork())<0){
        printf("fork error!");
        exit();
    }

    if(pid==0){
        // redirect(0,pd);
        close(pd[1]);
        childproccess(pd[0]);
        exit();
    }else{
        // redirect(1,pd);
        close(pd[0]);
        sendOriginalData(pd[1]);
        exit();
    }
}
