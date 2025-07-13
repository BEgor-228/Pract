#include <iostream>
#include <chrono>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
using namespace std::chrono;

// Тест скорости sem_wait/sem_post в одном процессе
void test_single_process() {
    sem_t sem;
    sem_init(&sem, 0, 1);  // Инициализация неименованного семафора

    const int iterations = 1000000;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        sem_wait(&sem);
        sem_post(&sem);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();

    cout << "Single process (unamed semaphore):\n";
    cout << "  Operations: " << iterations << "\n";
    cout << "  Total time: " << duration << " μs\n";
    cout << "  Time per operation: " << (double)duration / iterations << " μs\n\n";

    sem_destroy(&sem);
}

// Тест скорости между двумя процессами
void test_multi_process() {
    const char* sem_name = "/test_sem";
    sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 1);

    const int iterations = 100000;
    pid_t pid = fork();

    if (pid == 0) {  // Дочерний процесс
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            sem_wait(sem);
            sem_post(sem);
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start).count();
        cout << "Child process:\n";
        cout << "  Operations: " << iterations << "\n";
        cout << "  Total time: " << duration << " μs\n";
        cout << "  Time per operation: " << (double)duration / iterations << " μs\n";
        exit(0);
    } else {  // Родительский процесс
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            sem_wait(sem);
            sem_post(sem);
        }
        wait(nullptr);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start).count();
        cout << "Parent process:\n";
        cout << "  Operations: " << iterations << "\n";
        cout << "  Total time: " << duration << " μs\n";
        cout << "  Time per operation: " << (double)duration / iterations << " μs\n\n";
        sem_close(sem);
        sem_unlink(sem_name);
    }
}

int main() {
    cout << "--- POSIX Semaphore Performance Test ---\n\n";
    test_single_process();
    test_multi_process();
    return 0;
}