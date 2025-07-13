#include <stdio.h>
#include <unistd.h>
int main(int argc, char* argv[]){
    int A = 0,child_pid;
    printf ("Before fork():\nA = %d\n",A);
    child_pid = fork(); 
    if (child_pid == 0) {
        printf("It's the child process. Child_pid = %d; getpid = %d; A = %d\n",child_pid,getpid(),A); 
        A++;
        printf("It's the child process. Change A = %d\nPress enter..\n",A);
        getchar();
    }else if (child_pid > 0) {
        wait(child_pid);
        printf ("It's the main process. Child_pid = %d; getpid = %d; getppid = %d; A = %d\nPress enter..\n",child_pid,getpid(),getppid(),A);
        getchar();
    }
    else printf("fork error");
    return 0;
}