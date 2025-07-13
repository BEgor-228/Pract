#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>

#define M_PI 3.14159265358979323846

// Функция вычисления одного члена ряда Тейлора
double compute_taylor_term(double x, int k) {
    double term = x;
    for (int i = 1; i < k; i++) {
        term *= -x * x / ((2 * i) * (2 * i + 1));
    }
    return term;
}

int main() {
    int K, N, n;
    printf("Введите K (количество значений): ");
    scanf("%d", &K);
    printf("Введите N (параметр функции): ");
    scanf("%d", &N);
    printf("Введите n (количество членов ряда Тейлора): ");
    scanf("%d", &n);
    
    double y[K];
    FILE* file = fopen("output.txt", "w");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return 1;
    }
    
    for (int i = 0; i < K; i++) {
        double x = 2 * M_PI * i / N;
        int pid = fork();
        
        if (pid < 0) {
            perror("Ошибка создания процесса");
            return 1;
        } else if (pid == 0) { // Дочерний процесс
            double sum = 0.0;
            for (int k = 1; k <= n; k++) {
                sum += compute_taylor_term(x, k);
            }
            printf("Процесс %d: i=%d, PID=%d, y[%d]=%f\n", getpid(), i, getpid(), i, sum);
            exit(sum);
        }
    }
    
    for (int i = 0; i < K; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            y[i] = WEXITSTATUS(status);
            fprintf(file, "y[%d] = %f\n", i, y[i]);
        }
    }
    
    fclose(file);
    printf("Результаты записаны в output.txt\n");
    return 0;
}