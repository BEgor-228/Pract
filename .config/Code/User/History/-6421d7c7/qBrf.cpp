#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_REINDEERS 100
#define MAX_ELVES 100

int reindeer_count = 0;
int elf_count = 0;
int waiting_elves[MAX_ELVES];
int elves_in_cabinet = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elf_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;

int N, M, K, T;

void* reindeer(void* arg) {
    int id = *((int*)arg);
    while (1) {
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex);
        reindeer_count++;
        printf("%ld Олени %d приблизились. Ожидающие олени: %d. Ожидающие эльфы: %d\n", time(NULL), id, reindeer_count, elf_count);
        if (reindeer_count == N) {
            pthread_cond_signal(&santa_cond);
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5 + 1);
    }
}

void* elf(void* arg) {
    int id = *((int*)arg);
    while (1) {
        sleep(rand() % 5 + 1);
        pthread_mutex_lock(&mutex);
        if (elf_count < M) {
            waiting_elves[elf_count++] = id;
            printf("%ld Эльф %d приблизился. Ожидающие эльфы: %d. Ожидающие олени: %d\n", time(NULL), id, elf_count, reindeer_count);
            if (elf_count >= K && reindeer_count == 0) {
                pthread_cond_signal(&santa_cond);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5 + 1); 
    }
}

void* santa(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (reindeer_count < N && elf_count < K) {
            printf("%ld Санта спит.\n", time(NULL));
            pthread_cond_wait(&santa_cond, &mutex);
        }
        if (reindeer_count == N) {
            printf("%ld Санта запрягает оленей.\n", time(NULL));
            sleep(rand() % 5 + 1);
            printf("%ld Санта закончил доставку, олени свободны.\n", time(NULL));
            reindeer_count = 0;
            pthread_cond_broadcast(&reindeer_cond);
        } else if (elf_count >= K) {
            while (elves_in_cabinet < K && elf_count > 0) {
                int elf_id = waiting_elves[--elf_count];
                elves_in_cabinet++;
                printf("%ld Санта приглашает эльфа %d в кабинет. Эльфы, находящиеся в кабинете: %d\n", time(NULL), elf_id, elves_in_cabinet);
            }
            printf("%ld Санта встречается с эльфами.\n", time(NULL));
            sleep(T);
            while (elves_in_cabinet > 0) {
                elves_in_cabinet--;
                printf("%ld Санта выпускает эльфа из кабинета. Оставшиеся в кабинете эльфы: %d\n", time(NULL), elves_in_cabinet);
            }
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    srand(time(NULL));
    printf("Enter number of reindeers (N): ");
    scanf("%d", &N);
    printf("Enter number of elves (M): ");
    scanf("%d", &M);
    printf("Enter minimum elves to wake Santa (K): ");
    scanf("%d", &K);
    printf("Enter meeting time (T seconds): ");
    scanf("%d", &T);

    pthread_t santa_thread;
    pthread_t reindeer_threads[MAX_REINDEERS];
    pthread_t elf_threads[MAX_ELVES];
    int reindeer_ids[MAX_REINDEERS];
    int elf_ids[MAX_ELVES];

    pthread_create(&santa_thread, NULL, santa, NULL);

    for (int i = 0; i < N; i++) {
        reindeer_ids[i] = i + 1;
        pthread_create(&reindeer_threads[i], NULL, reindeer, &reindeer_ids[i]);
    }
    for (int i = 0; i < M; i++) {
        elf_ids[i] = i + 1;
        pthread_create(&elf_threads[i], NULL, elf, &elf_ids[i]);
    }

    pthread_join(santa_thread, NULL);
    return 0;
}