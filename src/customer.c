#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include "common.h"
#include "config_parser.h"

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    srand(time(NULL) ^ getpid());

    // Attach shared memory
    int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), 0666);
    if (shm_stats == -1) { perror("shmget stats"); exit(1); }

    SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);
    if (stats == (void*) -1) { perror("shmat stats"); exit(1); }

    // Semaphore
    int semid = semget(STATS_SEM_KEY, 1, 0666);
    if (semid == -1) { perror("semget stats"); exit(1); }

    // Check shutdown flag before doing anything
    sem_lock(semid);
    if (stats->shutdown_flag) {
        sem_unlock(semid);
        exit(0); // Exit early, no need to continue
    }
    sem_unlock(semid);

    // Message Queue
    int msqid = msgget(MSG_KEY_CUSTOMER, 0666);
    if (msqid == -1) { perror("msgget customer"); exit(1); }

    // Load config
    Config config;
    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load config.\n");
        exit(1);
    }

    // === Choose random item ===
    const char* item = final_product_names[rand() % FINAL_PRODUCT_COUNT];
    sleep(2);

    // Send request to seller
    CustomerMessage msg;
    msg.mtype = 1;
    msg.customer_pid = getpid();
    strncpy(msg.item, item, MAX_ITEM_NAME);

    if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("[Customer] msgsnd failed");
        exit(1);
    }

    // Wait for reply (with timeout loop)
    SellerReply reply;
    int received = 0;

    for (int waited = 0; waited < config.CUSTOMER_WAIT_TIME; waited++) {
        // Check shutdown during wait
        sem_lock(semid);
        
        if (stats->shutdown_flag) {
            sem_unlock(semid);
            exit(0); // Exit early if shutdown occurs while waiting
        }
        
        if (stats->complain_flag == 1) {
    if ((rand() % 100) < 50) {
        printf("[Customer %d] Panicked due to complaint and left.\n", getpid());
        sem_unlock(semid);
        exit(0);
    } else {
        stats->complain_flag = 0; // One customer calms down the rest
        
    }
}

        sem_unlock(semid);
        
        


        if (msgrcv(msqid, &reply, sizeof(reply) - sizeof(long), getpid(), IPC_NOWAIT) != -1) {
            received = 1;
            break;
        }
        sleep(1);
    }

    if (!received) {
        sem_lock(semid);
        stats->frustrated_customers++;
        printf("[Customer %d] Got frustrated waiting for %s. Total frustrated: %d\n",
               getpid(), item, stats->frustrated_customers);
        sem_unlock(semid);
    } else {
        printf("[Customer %d] Bought %s for $%d\n", getpid(), item, reply.price);
    }

    return 0;
}

