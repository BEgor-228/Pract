#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // Для fork(), getpid(), getppid()
#include <sys/wait.h> // Для wait()
#include <sys/time.h> // Для gettimeofday()
#include <time.h>     // Для localtime()

void print_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL); 

    struct tm *tm_info = localtime(&tv.tv_sec); 
    printf("%02d:%02d:%02d:%03ld", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec / 1000);
}

int main() {
    pid_t child1_pid, child2_pid;

    // Первый вызов fork()
    child1_pid = fork();

    if (child1_pid == 0) {
        // Первый дочерний процесс
        printf("Дочерний процесс 1: PID = %d, PPID = %d, Время: ", getpid(), getppid());
        print_current_time();
        printf("\n");
        exit(0); // Завершаем первый дочерний процесс
    } else if (child1_pid > 0) {
        // Родительский процесс
        // Второй вызов fork()
        child2_pid = fork();

        if (child2_pid == 0) {
            // Второй дочерний процесс
            printf("Дочерний процесс 2: PID = %d, PPID = %d, Время: ", getpid(), getppid());
            print_current_time();
            printf("\n");
            exit(0); // Завершаем второй дочерний процесс
        } else if (child2_pid > 0) {
            // Родительский процесс
            // Ожидаем завершения обоих дочерних процессов
            wait(NULL);
            wait(NULL);

            printf("Родительский процесс: PID = %d, PPID = %d, Время: ", getpid(), getppid());
            print_current_time();
            printf("\n");

            // Выполняем команду ps -x
            printf("Выполняем команду ps -x:\n");
            system("ps -x");

            // Поиск своих процессов в выводе ps -x
            printf("\nПоиск своих процессов в выводе ps -x:\n");
            char command[256];
            snprintf(command, sizeof(command), "ps -x | grep %d", getpid());
            system(command);
        } else {
            perror("Ошибка при втором вызове fork()");
            exit(1);
        }
    } else {
        perror("Ошибка при первом вызове fork()");
        exit(1);
    }

    return 0;
}