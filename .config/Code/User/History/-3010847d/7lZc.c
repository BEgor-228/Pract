#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // Для fork, sleep
#include <sys/wait.h> // Для wait

int main() {
    int child_pid = fork(); // Создание дочернего процесса

    if (child_pid == 0) { // Дочерний процесс
        printf("Дочерний процесс: Получение списка процессов...\n");

        // Открываем файл для записи
        FILE *file = fopen("process_list.txt", "w");
        if (file == NULL) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        // Выполняем команду ps aux и записываем вывод в файл
        fprintf(file, "Список процессов от дочернего процесса:\n");
        fflush(file); // Очищаем буфер, чтобы данные сразу попали в файл

        // Используем popen для выполнения команды и чтения её вывода
        FILE *ps_output = popen("ps aux", "r");
        if (ps_output == NULL) {
            perror("Ошибка выполнения popen");
            fclose(file);
            exit(1);
        }

        // Читаем вывод команды и записываем его в файл
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), ps_output) != NULL) {
            fprintf(file, "%s", buffer);
        }

        // Закрываем поток и файл
        pclose(ps_output);
        fclose(file);

        printf("Дочерний процесс: Список процессов записан в файл.\n");
        exit(0); // Завершение дочернего процесса
    }
    else if (child_pid > 0) { // Родительский процесс
        printf("Родительский процесс: Ожидание завершения дочернего процесса...\n");
        wait(NULL); // Ожидание завершения дочернего процесса

        printf("Родительский процесс: Дочерний процесс завершен. Получение списка процессов...\n");

        // Открываем файл для дописывания
        FILE *file = fopen("process_list.txt", "a");
        if (file == NULL) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        // Дописываем список процессов от родительского процесса
        fprintf(file, "\nСписок процессов от родительского процесса:\n");
        fflush(file); // Очищаем буфер

        // Используем popen для выполнения команды и чтения её вывода
        FILE *ps_output = popen("ps aux", "r");
        if (ps_output == NULL) {
            perror("Ошибка выполнения popen");
            fclose(file);
            exit(1);
        }

        // Читаем вывод команды и записываем его в файл
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), ps_output) != NULL) {
            fprintf(file, "%s", buffer);
        }

        // Закрываем поток и файл
        pclose(ps_output);
        fclose(file);

        printf("Родительский процесс: Список процессов дописан в файл.\n");
    }
    else { // Ошибка при вызове fork
        perror("Ошибка при вызове fork");
        exit(1);
    }

    return 0;
}