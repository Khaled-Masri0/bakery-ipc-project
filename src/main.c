#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "config_parser.h"

// === Globals for tracking child PIDs ===
#define MAX_CHILDREN 512
pid_t child_pids[MAX_CHILDREN];
int child_count = 0;

void track_child(pid_t pid) {
    if (child_count < MAX_CHILDREN) {
        child_pids[child_count++] = pid;
    }
}

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void initialize_semaphore(int semid) {
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl failed");
        exit(1);
    }
}

void cleanup_ipc() {
    shmctl(shmget(SHM_KEY_INGREDIENTS, 0, 0), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY_PASTE, 0, 0), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY_BREAD, 0, 0), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY_READY, 0, 0), IPC_RMID, NULL);
    shmctl(shmget(SHM_KEY_FINAL, 0, 0), IPC_RMID, NULL);
    shmctl(shmget(STATS_SHM_KEY, 0, 0), IPC_RMID, NULL);

    semctl(semget(SEM_KEY_INGREDIENTS, 1, 0), 0, IPC_RMID);
    semctl(semget(SEM_KEY_PASTE, 1, 0), 0, IPC_RMID);
    semctl(semget(SEM_KEY_BREAD, 1, 0), 0, IPC_RMID);
    semctl(semget(SEM_KEY_READY, 1, 0), 0, IPC_RMID);
    semctl(semget(SEM_KEY_FINAL, 1, 0), 0, IPC_RMID);
    semctl(semget(STATS_SEM_KEY, 1, 0), 0, IPC_RMID);

    msgctl(msgget(MSG_KEY_CUSTOMER, 0), IPC_RMID, NULL);

    printf("[Main] All IPC resources cleaned up.\n");
}

void fork_chefs(const Config* config) {
    const char* roles[] = {"rawbread", "paste", "cake", "sweet", "sweetp", "savoryp", "sandwich"};
    const int* counts[] = {
        &config->NUM_CHEFS_RAWBREAD,
        &config->NUM_CHEFS_PASTE,
        &config->NUM_CHEFS_CAKES,
        &config->NUM_CHEFS_SWEETS,
        &config->NUM_CHEFS_SWEET_PATISSERIES,
        &config->NUM_CHEFS_SAVORY_PATISSERIES,
        &config->NUM_CHEFS_SANDWICHES
    };

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < *counts[i]; j++) {
            pid_t pid = fork();
            if (pid == 0) execl("./chef", "chef", roles[i], NULL);
            else track_child(pid);
        }
        
    }
    printf("[Main] Chefs forked.\n");
}

void fork_bakers(const Config* config) {
    const char* teams[] = {"bread", "cakesweets", "patisseries"};
    const int* counts[] = {
        &config->NUM_BAKERS_BREAD,
        &config->NUM_BAKERS_CAKESWEETS,
        &config->NUM_BAKERS_PATISSERIES
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < *counts[i]; j++) {
            pid_t pid = fork();
            if (pid == 0) execl("./baker", "baker", teams[i], NULL);
            else track_child(pid);
        }
        
    }
    printf("[Main] Bakers forked.\n");
}

int main() {
    Config config;
    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load config.\n");
        exit(1);
    }

    // === Shared Memory and Semaphore Initialization ===
    int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), IPC_CREAT | 0666);
    SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);
    stats->total_profit = 0;
    stats->complaints = 0;
    stats->frustrated_customers = 0;
    stats->shutdown_flag = 0;
    stats->start_time = time(NULL);

    int sem_stats = semget(STATS_SEM_KEY, 1, IPC_CREAT | 0666);
    initialize_semaphore(sem_stats);

    // === Initialize Shared Memories ===
    int shm_ing = shmget(SHM_KEY_INGREDIENTS, sizeof(SharedBakery), IPC_CREAT | 0666);
    SharedBakery* ing = (SharedBakery*) shmat(shm_ing, NULL, 0);
    memset(ing, 0, sizeof(SharedBakery));

    int shm_paste = shmget(SHM_KEY_PASTE, sizeof(PasteStock), IPC_CREAT | 0666);
    PasteStock* paste = (PasteStock*) shmat(shm_paste, NULL, 0);
    memset(paste, 0, sizeof(PasteStock));

    int shm_bread = shmget(SHM_KEY_BREAD, sizeof(BreadStock), IPC_CREAT | 0666);
    BreadStock* bread = (BreadStock*) shmat(shm_bread, NULL, 0);
    memset(bread, 0, sizeof(BreadStock));

    int shm_ready = shmget(SHM_KEY_READY, sizeof(ReadyToBakeStock), IPC_CREAT | 0666);
    ReadyToBakeStock* ready = (ReadyToBakeStock*) shmat(shm_ready, NULL, 0);
    memset(ready, 0, sizeof(ReadyToBakeStock));

    int shm_final = shmget(SHM_KEY_FINAL, sizeof(FinalProductStock), IPC_CREAT | 0666);
    FinalProductStock* final = (FinalProductStock*) shmat(shm_final, NULL, 0);
    memset(final, 0, sizeof(FinalProductStock));

    // === Initialize Semaphores ===
    initialize_semaphore(semget(SEM_KEY_INGREDIENTS, 1, IPC_CREAT | 0666));
    initialize_semaphore(semget(SEM_KEY_PASTE, 1, IPC_CREAT | 0666));
    initialize_semaphore(semget(SEM_KEY_BREAD, 1, IPC_CREAT | 0666));
    initialize_semaphore(semget(SEM_KEY_READY, 1, IPC_CREAT | 0666));
    initialize_semaphore(semget(SEM_KEY_FINAL, 1, IPC_CREAT | 0666));

    for (int i = 0; i < INGREDIENT_COUNT; i++) ing->ingredients[i] = 5;
    paste->pastes[PASTE] = 5;
    bread->breads[BREAD] = 5;

    int msqid = msgget(MSG_KEY_CUSTOMER, IPC_CREAT | 0666);
    printf("[Main] Message queue and stats initialized.\n");

    // === Fork Suppliers ===
    for (int i = 0; i < config.NUM_SUPPLIERS; i++) {
        pid_t pid = fork();
        if (pid == 0) execl("./supplier", "supplier", NULL);
        else track_child(pid);
    }

    sleep(2);
    fork_chefs(&config);
    sleep(1);
    fork_bakers(&config);
    sleep(1);

    // Customer delay
    for (int i = config.CUSTOMER_ARRIVE_TIME; i > 0; i--) {
        printf("[Main] Customers arrive in %d seconds...\n", i);
        sleep(1);
    }

    // === Customer Forker Process ===
    pid_t customer_forker_pid = fork();
    if (customer_forker_pid == 0) {
        while (1) {
            sem_lock(sem_stats);
            if (stats->shutdown_flag) {
                sem_unlock(sem_stats);
                break;
            }
            sem_unlock(sem_stats);
            if (fork() == 0) {
                execl("./customer", "customer", NULL);
                perror("execl customer");
                exit(1);
            }
            sleep(config.CUSTOMER_EVERY_SECONDS);
        }
        exit(0);
    }
    track_child(customer_forker_pid);
    printf("[Main] Customers are arriving!\n");

    // === Fork Sellers ===
    for (int i = 0; i < config.NUM_SELLERS; i++) {
        pid_t pid = fork();
        if (pid == 0) execl("./seller", "seller", NULL);
        else track_child(pid);
    }
    printf("[Main] Sellers forked.\n");

    // === Monitoring Loop ===
    while (1) {
        sleep(2);
        sem_lock(sem_stats);
        time_t now = time(NULL);
        int runtime_minutes = (now - stats->start_time) / 60;

        if (stats->shutdown_flag ||
            stats->total_profit >= config.MAX_PROFIT ||
            stats->complaints >= config.MAX_CUSTOMER_COMPLAINTS ||
            stats->frustrated_customers >= config.MAX_FRUSTRATED_CUSTOMERS ||
            runtime_minutes >= config.MAX_RUNTIME_MINUTES) {

            stats->shutdown_flag = 1;
            sem_unlock(sem_stats);
            printf("[Main] Shutdown condition met. Terminating...\n");
            break;
        }
        sem_unlock(sem_stats);
    }

    // === Kill all children (just in case) ===
    for (int i = 0; i < child_count; i++) {
        kill(child_pids[i], SIGTERM);
    }

    // === Wait for cleanup ===
    while (wait(NULL) > 0);
    cleanup_ipc();
    printf("[Main] Simulation finished.\n");
    return 0;
}

