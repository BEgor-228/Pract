#include <iostream>
#include <semaphore.h>
#include <fcntl.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    sem_t* sem = sem_open("/sem_perf", O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    const int N = 100000;
    auto start = high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        sem_wait(sem);
        sem_post(sem);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    cout << "POSIX Semaphore: " << N << " operations took " << duration << " ms" << endl;

    sem_unlink("/sem_perf");
    return 0;
}