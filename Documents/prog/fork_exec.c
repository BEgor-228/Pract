#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/wait.h> 

int main(int argc, char* argv[]){
    int child_pid = fork(); 
    if (child_pid == 0) {
        printf("It's the children process:\n");
        char* arg_list[] = {"ls", "/home/bezrukavnikov", NULL}; 
        execvp("ls", arg_list); 
        perror("execvp"); 
        exit(1); 
    }
    else if (child_pid > 0) {
        wait(NULL); 
        printf("It's the main process. Press Enter..\n");
        getchar();
    }
    else{
        perror("fork"); 
        exit(1); 
    }
    return 0;
}