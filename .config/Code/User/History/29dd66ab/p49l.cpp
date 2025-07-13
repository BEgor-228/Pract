#include <iostream>
#include <fstream>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

const char* FILE_NAME = "shared_file1.txt";
const char* SEM_NAME = "/sem1";
const int MAX_LINES = 1000;
const int MAX_LINE_LEN = 128;

volatile sig_atomic_t start_flag = 0;

void signal_handler(int sig) {
    start_flag = 1;
}

void child_process(int index, char* shared_mem, sem_t* sem) {
    signal(SIGUSR1, signal_handler);
    while (!start_flag) pause(); // Ждём SIGUSR1

    for (int i = 0; i < 100; ++i) {
        sem_wait(sem);
        // Ищем первую свободную строку
        for (int j = 0; j < MAX_LINES; ++j) {
            char* line = shared_mem + j * MAX_LINE_LEN;
            if (line[0] == '\0') {
                auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                snprintf(line, MAX_LINE_LEN, "%03d %d %lld\n", j + 1, getpid(), now);
                break;
            }
        }
        sem_post(sem);
        usleep(1000);
    }
    exit(0);
}

int main() {
    // Создаём/обнуляем файл
    int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(fd, MAX_LINES * MAX_LINE_LEN);

    // Отображаем в память
    char* shared_mem = (char*) mmap(NULL, MAX_LINES * MAX_LINE_LEN,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Создаём семафор
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    // Создаём два дочерних процесса
    pid_t p1 = fork();
    if (p1 == 0) {
        child_process(1, shared_mem, sem);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        child_process(2, shared_mem, sem);
    }

    // Родитель ждёт немного и посылает SIGUSR1
    sleep(1);
    kill(p1, SIGUSR1);
    kill(p2, SIGUSR1);

    // Ожидаем заполнения части строк
    sleep(3);

    // Чтение первых 75 строк
    cout << "Первые 75 строк из файла:\n";
    sem_wait(sem);
    for (int i = 0; i < 75; ++i) {
        char* line = shared_mem + i * MAX_LINE_LEN;
        if (line[0] != '\0') {
            cout << "pid: " << getpid() << " строка: " << line;
        }
    }
    sem_post(sem);

    // Ждём завершения дочерних процессов
    wait(nullptr);
    wait(nullptr);

    // Очистка
    // sem_close(sem);
    // sem_unlink(SEM_NAME);
    // munmap(shared_mem, MAX_LINES * MAX_LINE_LEN);
    // close(fd);
    // unlink(FILE_NAME);

    return 0;
}