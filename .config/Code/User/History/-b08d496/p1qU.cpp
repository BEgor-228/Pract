// task1_file_semaphore.cpp
#include <iostream>
#include <fstream>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    const char* semName = "/sem1";
    sem_t* sem = sem_open(semName, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    const char* fileName = "shared_file.txt";
    ofstream file(fileName); // truncate if exists
    file.close();

    pid_t pid = fork();
    if (pid == 0) {
        // Child process: writer
        for (int i = 1; i <= 100; ++i) {
            sem_wait(sem);
            ofstream out(fileName, ios::app);
            auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            out << i << " " << getpid() << " " << now << "\n";
            out.close();
            sem_post(sem);
            usleep(1000); // 1 ms
        }
    } else {
        // Parent process: reader
        for (int i = 0; i < 100; ++i) {
            sem_wait(sem);
            ifstream in(fileName);
            string line;
            while (getline(in, line)) {
                cout << "pid " << pid << " " << line << endl;
            }
            in.close();
            sem_post(sem);
            usleep(5000);
        }
        wait(nullptr);
        sem_unlink(semName);
    }
    return 0;
}