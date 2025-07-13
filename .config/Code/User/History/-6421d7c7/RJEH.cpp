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
        sleep(rand() % 5 + 1); // Random walk time
        pthread_mutex_lock(&mutex);
        reindeer_count++;
        printf("%ld Reindeer %d approached. Waiting reindeers: %d. Waiting elves: %d\n", time(NULL), id, reindeer_count, elf_count);
        if (reindeer_count == N) {
            pthread_cond_signal(&santa_cond);
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5 + 1); // Random walk after delivery
    }
}

void* elf(void* arg) {
    int id = *((int*)arg);
    while (1) {
        sleep(rand() % 5 + 1); // Random work time
        pthread_mutex_lock(&mutex);
        if (elf_count < M) {
            waiting_elves[elf_count++] = id;
            printf("%ld Elf %d approached. Waiting elves: %d. Waiting reindeers: %d\n", time(NULL), id, elf_count, reindeer_count);
            if (elf_count >= K && reindeer_count == 0) {
                pthread_cond_signal(&santa_cond);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 5 + 1); // Random work after meeting
    }
}

void* santa(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (reindeer_count < N && elf_count < K) {
            printf("%ld Santa is sleeping.\n", time(NULL));
            pthread_cond_wait(&santa_cond, &mutex);
        }
        if (reindeer_count == N) {
            printf("%ld Santa is hitching reindeers.\n", time(NULL));
            sleep(rand() % 5 + 1); // Random delivery time
            printf("%ld Santa finished delivery, reindeers are free.\n", time(NULL));
            reindeer_count = 0;
            pthread_cond_broadcast(&reindeer_cond);
        } else if (elf_count >= K) {
            while (elves_in_cabinet < K && elf_count > 0) {
                int elf_id = waiting_elves[--elf_count];
                elves_in_cabinet++;
                printf("%ld Santa invites Elf %d to cabinet. Elves in cabinet: %d\n", time(NULL), elf_id, elves_in_cabinet);
            }
            printf("%ld Santa starts meeting with elves.\n", time(NULL));
            sleep(T); // Meeting time
            while (elves_in_cabinet > 0) {
                elves_in_cabinet--;
                printf("%ld Santa releases an elf from cabinet. Remaining in cabinet: %d\n", time(NULL), elves_in_cabinet);
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