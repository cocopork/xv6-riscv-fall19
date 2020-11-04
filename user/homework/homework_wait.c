/* homework_wait.c */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#define N 10
void homework_wait() {
    pid_t pid[N];
    int i, child_status;
    pid_t w_child_pid;
    for (i = 0; i < N; i++) {
        if ((pid[i] = fork()) == 0) {
            // if(i>0){
            //     //printf("%d : %d is waiting for %d\n",getpid(),i,pid[i-1]);
            //     w_child_pid = waitpid(pid[i-1],0,0);
            //     //w_child_pid = waitpid(0,0,0,0);
            //     if(w_child_pid==-1){
            //         printf("%d : exit wrong\n",getpid());
            //         exit(-1);
            //     }
            // }
            sleep(i);
            printf("%d : 100+%d=%d\n",getpid(),i,100+i);
            exit(100+i); /* Child */
        }
        
    }
    printf("hello!\n");
    for (i = 0; i < N; i++) { /* Parent */
        pid_t wpid = wait(&child_status);
        if (WIFEXITED(child_status))
            printf("Child %d terminated with exit status %d\n",
            wpid, WEXITSTATUS(child_status));
        else
            printf("Child %d terminate abnormally\n", wpid);
    }
}
int main(){
    homework_wait();
    exit(0);
}
