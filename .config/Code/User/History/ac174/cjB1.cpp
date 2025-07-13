#include <stdio.h>
#include <unistd.h>
int main(){
    printf("Process ID: %d\n", (int) getpid());
    printf("Parent process ID: %d\n", (int) getppid());
    getchar();
    return 0;
}