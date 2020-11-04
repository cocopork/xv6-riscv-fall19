#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char* argv[]){
    int parent_fd[2];
    int child_fd[2];
    char parent_buf[256];
    char child_buf[256];
    int pid;
    if(argc>=2){
        printf("too much agrvs!");
        exit(1);
    }

    if(pipe(parent_fd)<0){
        printf("parent pipe error!/n");
        exit(1);
    }
    if(pipe(child_fd)<0){
        printf("child pipe error!/n");
        exit(1);
    }
    if((pid=fork())<0){
        printf("fork error!");
        exit(1);
    }
    //child proccess
    if(pid==0){
        close(parent_fd[1]);
        close(child_fd[0]);
        write(child_fd[1],"pong",strlen("pong"));
        memset(child_buf,0,sizeof(child_buf));
        read(parent_fd[0],child_buf,sizeof(child_buf));
        printf("%d: received %s\n",getpid(),child_buf);
        exit(0);
    }
    //parent proccess

    close(parent_fd[0]);
    close(child_fd[1]);
    write(parent_fd[1],"ping",strlen("ping"));
    memset(parent_buf,0,sizeof(parent_buf));
    read(child_fd[0],parent_buf,sizeof(parent_buf));
    printf("%d: received %s\n",getpid(),parent_buf);//use getPid() return proccess id
    exit(0);


}