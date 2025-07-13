#include <stdio.h>
#include <unistd.h>
int main(int argc, char* argv[], char* envp[]){
    char program [] = "ls";
    char* arg_list [10] = {"ls","/home/user"};
    execvp (program,arg_list);
    printf("Error on program start");
    return 123;
}