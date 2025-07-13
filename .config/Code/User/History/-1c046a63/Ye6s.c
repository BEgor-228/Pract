#include <stdio.h>
#include <stdlib.h>
#include <signal.h>   
#include <unistd.h>   
#include <sys/wait.h> 

int main(int argc, char* argv[]){
    int child_pid = fork(); 
    if (child_pid == 0) {
        printf("It's the children process:\n");
        sleep(1); 
        char* arg_list[] = {"top", NULL}; /
        execvp("top", arg_list); 
        perror("execvp"); 
        exit(1); 
    }
    else if (child_pid > 0){
        sleep(7); 
        int k = kill(child_pid, SIGTERM); 
        if (k == -1) {perror("kill"); }
        wait(NULL); 
        printf("Children process terminated..\n");
        printf("It's the main process. Press Enter..\n");
        getchar();
    }
    else {perror("fork"); exit(1); }
    return 0;
}