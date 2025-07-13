// task4_sysv_perf.cpp
#include <iostream>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    key_t key = ftok("/tmp", 'S');
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    semctl(semid, 0, SETVAL, 1);

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock = {0, 1, 0};

    const int N = 100000;
    auto start = high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        semop(semid, &lock, 1);
        semop(semid, &unlock, 1);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    cout << "System V Semaphore: " << N << " operations took " << duration << " ms" << endl;

    semctl(semid, 0, IPC_RMID);
    return 0;
}