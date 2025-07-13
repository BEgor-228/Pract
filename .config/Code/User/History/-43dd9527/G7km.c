#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[]){
    int child_pid = fork();
    if (child_pid == 0){
        printf("It's the children process:\n");
        char* arg_list [] = {"ls","/home/bezrukavnikov"};
        execvp ("ls", arg_list);
        exit(1);
    }
    else{
        wait(child_pid);
        printf("It's the main process. Press Enter..\n");
        getchar();
    }
    return 0;
}