#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // Для fork, execvp, wait
#include <sys/wait.h> // Для wait

int main(int argc, char* argv[])
{
    int child_pid = fork(); // Создание дочернего процесса

    if (child_pid == 0) // Дочерний процесс
    {
        printf("It's the children process:\n");
        char* arg_list[] = {"ls", "/home/bezrukavnikov", NULL}; // Аргументы для execvp
        execvp("ls", arg_list); // Замена текущего процесса на "ls"
        // Если execvp вернул управление, значит произошла ошибка
        perror("execvp"); // Вывод ошибки
        exit(1); // Завершение дочернего процесса с кодом ошибки
    }
    else if (child_pid > 0) // Родительский процесс
    {
        wait(NULL); // Ожидание завершения дочернего процесса
        printf("It's the main process. Press Enter..\n");
        getchar();
    }
    else // Ошибка при вызове fork
    {
        perror("fork"); // Вывод ошибки
        exit(1); // Завершение программы с кодом ошибки
    }

    return 0;
}