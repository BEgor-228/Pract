#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

// Структура для передачи данных в поток
typedef struct {
    int i;          // Индекс значения y[i]
    int N;          // Параметр N
    int n;          // Количество членов ряда Тейлора
    double result;  // Результат вычисления ряда Тейлора
} ThreadData;

// Функция для вычисления ряда Тейлора для sin(x)
void* compute_taylor_series(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    double x = 2 * M_PI * data->i / data->N;  // Вычисляем x = 2*pi*i/N
    double term = x;  // Первый член ряда Тейлора
    double sum = term;

    for (int k = 1; k < data->n; k++) {
        term *= -x * x / ((2 * k) * (2 * k + 1));  // Вычисляем следующий член ряда
        sum += term;
    }

    data->result = sum;  // Сохраняем результат
    printf("Поток для i = %d: PID = %ld, Результат = %f\n", data->i, (long)pthread_self(), data->result);
    pthread_exit(NULL);
}

int main() {
    int K, N, n;
    printf("Введите K (количество значений): ");
    scanf("%d", &K);
    printf("Введите N (параметр функции): ");
    scanf("%d", &N);
    printf("Введите n (количество членов ряда Тейлора): ");
    scanf("%d", &n);

    // Массив для хранения результатов y[i]
    double y[K];

    // Массив потоков и данных для потоков
    pthread_t threads[K];
    ThreadData thread_data[K];

    // Создаем потоки для вычисления каждого значения y[i]
    for (int i = 0; i < K; i++) {
        thread_data[i].i = i;
        thread_data[i].N = N;
        thread_data[i].n = n;
        pthread_create(&threads[i], NULL, compute_taylor_series, (void*)&thread_data[i]);
    }

    // Ожидаем завершения всех потоков
    for (int i = 0; i < K; i++) {
        pthread_join(threads[i], NULL);
        y[i] = thread_data[i].result;  // Сохраняем результат в массив y
    }

    // Записываем результаты в файл
    FILE* file = fopen("output.txt", "w");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return 1;
    }

    for (int i = 0; i < K; i++) {
        fprintf(file, "y[%d] = %f\n", i, y[i]);
    }

    fclose(file);
    printf("Результаты записаны в файл output.txt\n");

    return 0;
}