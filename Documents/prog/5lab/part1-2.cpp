#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();

    if (pid == 0) {
        close(pipefd[0]);
        for (int i = 1; i <= 100; ++i) {
            auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            string msg = to_string(i) + " " + to_string(getpid()) + " " + to_string(now) + "\n";
            write(pipefd[1], msg.c_str(), msg.size());
            usleep(1000);
        }
        close(pipefd[1]);
    } else {
        // Parent: read from pipe
        close(pipefd[1]);
        char buffer[256];
        ssize_t bytes;
        while ((bytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = '\0';
            cout << "pid " << pid << " " << buffer;
        }
        close(pipefd[0]);
        wait(nullptr);
    }
    return 0;
}