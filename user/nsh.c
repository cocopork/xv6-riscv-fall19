#include "../kernel/types.h"
#include "user.h"
#include "../kernel/fcntl.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3

//constant
#define BSIZE 100
#define ARGSIZE 32

char *start = 0;
char *next = 0;
char *temp = 0;
int fd = 0;
static char buf[BSIZE];
char* gettoken(char* current,char border);
void redir(int discriptor_to_close,int pd[]);
void handle(char *cmd);
void execcmd();
int getcmd(char* buf,int nbuf);

inline int print(int fd1, char *str)
{
    return write(fd1, str, strlen(str));
}
//global variable

int main(void){
    
    //int parent_pid = getpid();
    //fprintf(2,"root pid %d\n",parent_pid);

    while((fd = open("console", O_RDWR)) >= 0){
        if(fd >= 3){
        close(fd);
        break;
        }
    }
    
    while(getcmd(buf,sizeof(buf))>=0){
        
        //change directory
        if(buf[0]=='c'&&buf[1]=='d'&&buf[2]==' '){
            buf[strlen(buf)-1]=0;
            if(chdir(buf+3)<0){
                fprintf(2,"no such file or directory : %s\n",buf+3);
                continue;
            }
        }
        else{
            temp = strchr(buf,'\n');
            *temp = '\0';
            //子进程执行
            if(fork()==0){
                //fprintf(2, "%d -> %d source\n", parent_pid, getpid());
                start = buf;
                next = gettoken(start,'|');   
                execcmd();
            }
            //父进程等待
            else{
                //fprintf(2,"pid %d source\n", getpid());
                wait(0);
            }
        }       
    }
    exit(0);
}


int getcmd(char *buf,int nbuf){
    //fprintf(1,"@ ");
    print(1,"@ ");
    
    memset(buf,0,nbuf);
    buf[0]=0;
    gets(buf,nbuf);
    if(buf[0]==0){
        return -1;
    }
    return 0;
}

char *gettoken(char* current,char border){
    //fprintf(2,"now in pid %d!\n",getpid());
    //fprintf(2,"%s",current);
    while(*current!='\0'&&*current!=border){
        current++;
    }
    
    if(*current=='\0')
        return 0;
    *current = '\0';
    return current+1;

}

void redir(int discriptor_to_close,int pd[]){
    close(discriptor_to_close);
    dup(pd[discriptor_to_close]);
    close(pd[0]);
    close(pd[1]);
}

char* deleteBlank(char *cmd){
    char *cmd_s=cmd;//头
    char *cmd_e=cmd;//尾
    while(*cmd_e){
        cmd_e++;
    }
    while(*cmd_s==' ')
        cmd_s++;
    while(*cmd_e==' ')
        cmd_e--;
    *(cmd_e+1) = '\0';

    return cmd_s;
}

void handle(char *cmd){
    char buf_args[ARGSIZE][ARGSIZE];
    //char *bufArgsPointer[ARGSIZE];
    int argcNum = 0;
    
    char *p;
    
    char *bufArgsPointer;
    int inputRedirIndex=0;//输入重定向
    int outputRedirIndex=0;//输出重定向
    
    char *argToPass[ARGSIZE];
    int argcPassNum = 0;
    int i;

    cmd = deleteBlank(cmd);
    bufArgsPointer = buf_args[argcNum];

    p = cmd;
    while(*p){
        //遇到中间隔断或者结束
        if(*p==' '||*p=='\n'){
            *bufArgsPointer = '\0';
            argcNum++;
            bufArgsPointer = buf_args[argcNum];
        }
        // > 或 < 或参数本身
        else{
            if(*p=='<')
                inputRedirIndex = argcNum+1;
                
            if(*p=='>')
                outputRedirIndex = argcNum+1;
            //保存一个 < 或 >
            *bufArgsPointer = *p;
            bufArgsPointer++;
        }
        p++;
    }
    *bufArgsPointer = '\0';
    argcNum++;
    bufArgsPointer = buf_args[argcNum];
    *bufArgsPointer='\0';

    /*
    print(1,"=======  buf_args  =======\n");
    for(i=0;i<=argcNum;i++){
        printf("%d : %s\n",i,buf_args[i]);
    }
    print(1,"==============\n");
    */

    if(inputRedirIndex){
        close(0);
        open(buf_args[inputRedirIndex],O_RDONLY);
    }

    if(outputRedirIndex){
        close(1);
        open(buf_args[outputRedirIndex],O_WRONLY|O_CREATE);
    }

    for(i=0;i<argcNum;i++){
        if(i==inputRedirIndex-1) i = i+2;
        if(i==outputRedirIndex-1) i = i+2;
        if(strcmp(buf_args[i],"\0")==0){
            argToPass[argcPassNum++] = 0;
        }
        else{
            argToPass[argcPassNum++]=buf_args[i];
        }
        
    }
    argToPass[argcPassNum] = 0;
    /*
    fprintf(2,"=======  argToPass %d  =======\n",argcNum);
    for(int i=0;i<=argcPassNum;i++){
        fprintf(2,"%d : %s\n",i,argToPass[i]);
    }
    print(1,"==============\n");
    */
    if(fork()){
        wait(0);
    }
    else{
        exec(argToPass[0],argToPass);
    }

}

void execcmd(){
    if(start){
        int pd[2];
        //int parent_pid = getpid();
        if(pipe(pd)<0){
            fprintf(2,"pipe error, in pid %d!\n",getpid());
        }

        //处理start指令
        if(fork()==0){
            //fprintf(2, "%d -> %d source\n", parent_pid, getpid());
            //后面还有指令就重定向
            if(next){
                redir(1,pd);
            }
            handle(start);
        }
        //处理next指令
        else if(fork()==0){
            //fprintf(2, "%d -> %d source\n", parent_pid, getpid());
            if(next){
                redir(0,pd);
                start = next;
                next = gettoken(start,'|');
                execcmd(start);
                
            }
        }

        close(pd[0]);
        close(pd[1]);
        wait(0);
        wait(0);

    }
    exit(0);
}