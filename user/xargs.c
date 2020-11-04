#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]){
    int preInputLength;
    char preInputLine[MAXARG];
    char buf[MAXARG];//临时保存输入的一行
    int bufIndex=0;
    int i;
    // int k;//for test
    char *argHeader[MAXARG];//保存输入行中，所有参数的首地址
    int argHeaderIndex=0;
    char *p=buf;//保存输入行中，一个参数的首地址
    int pid;
    //先保存xargv的参数
    for(i=1;i<argc;i++){
        argHeader[argHeaderIndex]=argv[i];
        argHeaderIndex++;
    }
    //再保存前面输入的参数
    while((preInputLength=read(0,preInputLine,sizeof(preInputLine)))>0){//从标准输入中读取字符
        
        for(i=0;i<preInputLength;i++){
            //若读完一行输入参数
            if(preInputLine[i]=='\n'){
                buf[bufIndex]=0;
                bufIndex=0;
                argHeader[argHeaderIndex] = p;
                p = buf;//读完一行，重回buf头
                argHeader[argHeaderIndex+1]=0;//参数最后一行必须是0
                argHeaderIndex = argc-1;//重回xargv参数的下一位，准备另一行代码
                if((pid=fork())<0){
                    fprintf(2,"fork fail!");
                    exit();
                }
                if(pid==0){
                    //测试部分，输入显示
                    // printf("=====================%s \n",argv[1]);
                    // for(k=0;k<sizeof(argHeader)&&(strcmp(argHeader[k],0)!=0);k++){
                    //     printf("%s ",argHeader[k]);
                    // }
                    // printf("\n");

                    exec(argv[1],argHeader);
                }
                wait();
            }
            //若读到空格，说明读完了一个参数
            else if(preInputLine[i]==' '){
                buf[bufIndex]=0;//对应buf保存为\0
                bufIndex++;
                argHeader[argHeaderIndex] = p;
                argHeaderIndex++;
                p = &buf[bufIndex];

            }
            //其他字符
            else{
                buf[bufIndex] = preInputLine[i];
                bufIndex++;
            }


        }
    }
    
    exit();
}