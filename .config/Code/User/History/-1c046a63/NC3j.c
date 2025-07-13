#include <stdio.h>
#include <stdlib.h>
#include <signal.h>   // Для kill
#include <unistd.h>   // Для fork, sleep, execvp
#include <sys/wait.h> // Для wait

int main(int argc, char* argv[])
{
    int child_pid = fork(); // Создание дочернего процесса

    if (child_pid == 0) // Дочерний процесс
    {
        printf("It's the children process:\n");
        sleep(1); // Пауза 1 секунда
        char* arg_list[] = {"top", NULL}; // Аргументы для execvp
        execvp("top", arg_list); // Замена текущего процесса на "top"
        // Если execvp вернул управление, значит произошла ошибка
        perror("execvp"); // Вывод ошибки
        exit(1); // Завершение дочернего процесса с кодом ошибки
    }
    else if (child_pid > 0) // Родительский процесс
    {
        sleep(7); // Пауза 7 секунд
        int k = kill(child_pid, SIGTERM); // Отправка сигнала SIGTERM дочернему процессу
        if (k == -1) // Проверка на ошибку
        {
            perror("kill"); // Вывод ошибки
        }
        wait(NULL); // Ожидание завершения дочернего процесса
        printf("Children process terminated..\n");
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