#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int main(int argc, char* argv[]){
    int child_pid = fork();
    if (child_pid == 0){
        printf("It's the children process:\n");
        sleep(1);
        char* arg_list[] = {""};
        execvp ("top", arg_list);
        exit(1);
    }
    else{
        int k;
        sleep(7);
        k = kill (child_pid,SIGTERM);
        wait(child_pid);
        printf("Children process terminated..\n");
        printf("It's the main process. Press Enter..\n");
        getchar();
    }
    return 0;
}