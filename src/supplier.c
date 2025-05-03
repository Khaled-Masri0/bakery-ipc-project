#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#include "common.h"
#include "config_parser.h"

// Semaphore lock (P)
void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

// Semaphore unlock (V)
void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

// Random number in range [min, max]
int random_between(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main() {
    srand(time(NULL) ^ getpid());

    // Attach to shared memory for ingredients
    int shmid = shmget(SHM_KEY_INGREDIENTS, sizeof(SharedBakery), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    SharedBakery* bakery = (SharedBakery*) shmat(shmid, NULL, 0);
    if (bakery == (void*) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Attach to semaphore for ingredients
    int semid = semget(SEM_KEY_INGREDIENTS, 1, 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }
    
    // Attach to shared memory for stats
int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), 0666);
if (shm_stats == -1) { perror("shmget stats"); exit(1); }

SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);
if (stats == (void*) -1) { perror("shmat stats"); exit(1); }

// Attach to semaphore for stats
int sem_stats = semget(STATS_SEM_KEY, 1, 0666);
if (sem_stats == -1) { perror("semget stats"); exit(1); }


    // Load configuration file
    Config config;
    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        exit(1);
    }

    printf("[Supplier %d] Started successfully.\n", getpid());

    while (1) {
    sem_lock(sem_stats);
if (stats->shutdown_flag) {
    sem_unlock(sem_stats);
    printf("[Supplier %d] Shutdown detected. Exiting.\n", getpid());
    break;
}
sem_unlock(sem_stats);

        sem_lock(semid);

        // Randomly pick an ingredient
        int idx = rand() % INGREDIENT_COUNT;

        int current_quantity = bakery->ingredients[idx];
        int min_required = 0;
        int max_allowed = 0;

        switch (idx) {
            case WHEAT:
                min_required = config.WHEAT_MIN;
                max_allowed = config.WHEAT_MAX;
                break;
            case YEAST:
                min_required = config.YEAST_MIN;
                max_allowed = config.YEAST_MAX;
                break;
            case BUTTER:
                min_required = config.BUTTER_MIN;
                max_allowed = config.BUTTER_MAX;
                break;
            case MILK:
                min_required = config.MILK_MIN;
                max_allowed = config.MILK_MAX;
                break;
            case SUGAR_SALT:
                min_required = config.SUGAR_SALT_MIN;
                max_allowed = config.SUGAR_SALT_MAX;
                break;
            case SWEET_ITEMS:
                min_required = config.SWEET_ITEMS_MIN;
                max_allowed = config.SWEET_ITEMS_MAX;
                break;
            case CHEESE:
            case SALAMI:
                min_required = config.CHEESE_SALAMI_MIN;
                max_allowed = config.CHEESE_SALAMI_MAX;
                break;
        }

        if (current_quantity < min_required) {
            int refill = random_between(1, max_allowed - current_quantity);
            bakery->ingredients[idx] += refill;

            printf("[Supplier %d] Added %d units of %s (now %d)\n",
                   getpid(), refill, ingredient_names[idx], bakery->ingredients[idx]);
        }

        sem_unlock(semid);

        sleep(2);
    }

    return 0;
}
