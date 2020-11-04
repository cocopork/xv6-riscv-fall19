#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
void handler(){
    fprintf(stdout,"received a signal\n");
    exit(0);
}

int main()
{
    int child_pid;
    static sem_t mutex;
    if((child_pid=fork())==0)
    {   
        signal(SIGUSR1,handler);
        while(1) ;
        printf("forbidden zone\n");
        exit(0);
    }
    else
    {
        while(getc(stdin))
            {
                kill(child_pid,SIGUSR1);
                wait(0);
                exit(0);
            }
    }
}