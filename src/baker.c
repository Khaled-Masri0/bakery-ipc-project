#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"
#include "config_parser.h"

typedef enum {
    TEAM_BREAD,
    TEAM_CAKESWEETS,
    TEAM_PATISSERIES,
    TEAM_UNKNOWN
} BakerTeam;

BakerTeam get_team_type(const char* name) {
    if (strcmp(name, "bread") == 0) return TEAM_BREAD;
    if (strcmp(name, "cakesweets") == 0) return TEAM_CAKESWEETS;
    if (strcmp(name, "patisseries") == 0) return TEAM_PATISSERIES;
    return TEAM_UNKNOWN;
}

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int get_bake_time_for(int product_type) {
    switch (product_type) {
        case BREAD: return 4;
        case CAKE: return 6;
        case SWEET: return 5;
        case SWEET_PATISSERIE: return 7;
        case SAVORY_PATISSERIE: return 7;
        default: return 3;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [bread|cakesweets|patisseries]\n", argv[0]);
        return 1;
    }

    BakerTeam team = get_team_type(argv[1]);
    if (team == TEAM_UNKNOWN) {
        fprintf(stderr, "Unknown team type: %s\n", argv[1]);
        return 1;
    }

    srand(time(NULL) ^ getpid());
    printf("[Baker %d] Starting as team: %s\n", getpid(), argv[1]);

    Config config;
    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "[Baker %d] Failed to load config file.\n", getpid());
        return 1;
    }

    int shm_ready = shmget(SHM_KEY_READY, sizeof(ReadyToBakeStock), 0666);
    ReadyToBakeStock* ready = (ReadyToBakeStock*) shmat(shm_ready, NULL, 0);
    int sem_ready = semget(SEM_KEY_READY, 1, 0666);

    int shm_final = -1, sem_final = -1;
    FinalProductStock* final = NULL;

    int shm_bread = -1, sem_bread = -1;
    BreadStock* bread = NULL;

    int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), 0666);
    SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);
    int sem_stats = semget(STATS_SEM_KEY, 1, 0666);

    if (team == TEAM_BREAD) {
        shm_bread = shmget(SHM_KEY_BREAD, sizeof(BreadStock), 0666);
        bread = (BreadStock*) shmat(shm_bread, NULL, 0);
        sem_bread = semget(SEM_KEY_BREAD, 1, 0666);
    } else {
        shm_final = shmget(SHM_KEY_FINAL, sizeof(FinalProductStock), 0666);
        final = (FinalProductStock*) shmat(shm_final, NULL, 0);
        sem_final = semget(SEM_KEY_FINAL, 1, 0666);
    }

    while (1) {
        // Check shutdown
        sem_lock(sem_stats);
        if (stats->shutdown_flag) {
            sem_unlock(sem_stats);
            printf("[Baker %d] Shutdown flag detected. Exiting.\n", getpid());
            break;
        }
        sem_unlock(sem_stats);

        sem_lock(sem_ready);

        if (team == TEAM_BREAD) {
            if (ready->ready_products[RAW_BREAD] > 0) {
                ready->ready_products[RAW_BREAD]--;
                sem_unlock(sem_ready);

                sleep(get_bake_time_for(BREAD));

                sem_lock(sem_bread);
                if (bread->breads[BREAD] < config.MAX_BREAD) {
                    bread->breads[BREAD]++;
                    printf("[Baker %d] Baked Bread. Total: %d\n", getpid(), bread->breads[BREAD]);
                } else {
                    printf("[Baker %d] Max Bread limit reached. Skipping.\n", getpid());
                }
                sem_unlock(sem_bread);
            } else {
                printf("[Baker %d] No raw bread to bake.\n", getpid());
                sem_unlock(sem_ready);
            }
        }

        else if (team == TEAM_CAKESWEETS) {
            if (ready->ready_products[RAW_CAKE] > 0) {
                ready->ready_products[RAW_CAKE]--;
                sem_unlock(sem_ready);
                sleep(get_bake_time_for(CAKE));

                sem_lock(sem_final);
                if (final->final_products[CAKE] < config.MAX_CAKE) {
                    final->final_products[CAKE]++;
                    printf("[Baker %d] Baked Cake. Total: %d\n", getpid(), final->final_products[CAKE]);
                } else {
                    printf("[Baker %d] Max Cake limit reached. Skipping.\n", getpid());
                }
                sem_unlock(sem_final);
            } else {
                sem_unlock(sem_ready);
                printf("[Baker %d] No raw cake to bake.\n", getpid());
            }

            sem_lock(sem_ready);
            if (ready->ready_products[RAW_SWEET] > 0) {
                ready->ready_products[RAW_SWEET]--;
                sem_unlock(sem_ready);
                sleep(get_bake_time_for(SWEET));

                sem_lock(sem_final);
                if (final->final_products[SWEET] < config.MAX_SWEET) {
                    final->final_products[SWEET]++;
                    printf("[Baker %d] Baked Sweet. Total: %d\n", getpid(), final->final_products[SWEET]);
                } else {
                    printf("[Baker %d] Max Sweet limit reached. Skipping.\n", getpid());
                }
                sem_unlock(sem_final);
            } else {
                sem_unlock(sem_ready);
                printf("[Baker %d] No raw sweet to bake.\n", getpid());
            }
        }

        else if (team == TEAM_PATISSERIES) {
            sem_lock(sem_ready);
            if (ready->ready_products[RAW_SWEET_PATISSERIE] > 0) {
                ready->ready_products[RAW_SWEET_PATISSERIE]--;
                sem_unlock(sem_ready);
                sleep(get_bake_time_for(SWEET_PATISSERIE));

                sem_lock(sem_final);
                if (final->final_products[SWEET_PATISSERIE] < config.MAX_SWEET_PATISSERIE) {
                    final->final_products[SWEET_PATISSERIE]++;
                    printf("[Baker %d] Baked Sweet Patisserie. Total: %d\n", getpid(), final->final_products[SWEET_PATISSERIE]);
                } else {
                    printf("[Baker %d] Max Sweet Patisserie limit reached. Skipping.\n", getpid());
                }
                sem_unlock(sem_final);
            } else {
                sem_unlock(sem_ready);
                printf("[Baker %d] No raw sweet patisseries to bake.\n", getpid());
            }

            sem_lock(sem_ready);
            if (ready->ready_products[RAW_SAVORY_PATISSERIE] > 0) {
                ready->ready_products[RAW_SAVORY_PATISSERIE]--;
                sem_unlock(sem_ready);
                sleep(get_bake_time_for(SAVORY_PATISSERIE));

                sem_lock(sem_final);
                if (final->final_products[SAVORY_PATISSERIE] < config.MAX_SAVORY_PATISSERIE) {
                    final->final_products[SAVORY_PATISSERIE]++;
                    printf("[Baker %d] Baked Savory Patisserie. Total: %d\n", getpid(), final->final_products[SAVORY_PATISSERIE]);
                } else {
                    printf("[Baker %d] Max Savory Patisserie limit reached. Skipping.\n", getpid());
                }
                sem_unlock(sem_final);
            } else {
                sem_unlock(sem_ready);
                printf("[Baker %d] No raw savory patisseries to bake.\n", getpid());
            }
        }
        

        sleep(2); 
    }

    return 0;
}

