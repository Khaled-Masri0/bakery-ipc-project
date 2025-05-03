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

int get_item_index(const char* item) {
    if (strcmp(item, "Sandwich") == 0) return SANDWICH;
    if (strcmp(item, "Cake") == 0) return CAKE;
    if (strcmp(item, "Sweet") == 0) return SWEET;
    if (strcmp(item, "Sweet Patisserie") == 0) return SWEET_PATISSERIE;
    if (strcmp(item, "Savory Patisserie") == 0) return SAVORY_PATISSERIE;
    return -1;
}

int get_item_price(const char* item, const Config* config) {
    if (strcmp(item, "Sandwich") == 0) return config->PRICE_SANDWICH;
    if (strcmp(item, "Cake") == 0) return config->PRICE_CAKE;
    if (strcmp(item, "Sweet") == 0) return config->PRICE_SWEET;
    if (strcmp(item, "Sweet Patisserie") == 0) return config->PRICE_SWEET_PATISSERIE;
    if (strcmp(item, "Savory Patisserie") == 0) return config->PRICE_SAVORY_PATISSERIE;
    return 0;
}

int main() {
    srand(time(NULL) ^ getpid());

    // Attach shared memory
    int shm_final = shmget(SHM_KEY_FINAL, sizeof(FinalProductStock), 0666);
    FinalProductStock* final = (FinalProductStock*) shmat(shm_final, NULL, 0);

    int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), 0666);
    SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);

    // Semaphores
    int sem_final = semget(SEM_KEY_FINAL, 1, 0666);
    int sem_stats = semget(STATS_SEM_KEY, 1, 0666);

    // Config
    Config config;
    if (load_config("config/config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load config.\n");
        exit(1);
    }

    // Message Queue
    int msqid = msgget(MSG_KEY_CUSTOMER, 0666);
    printf("[Seller %d] Ready to serve customers.\n", getpid());

    while (1) {
        CustomerMessage request;
        if (msgrcv(msqid, &request, sizeof(request) - sizeof(long), 1, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        // Check shutdown
        sem_lock(sem_stats);
        if (stats->shutdown_flag) {
            sem_unlock(sem_stats);
            printf("[Seller %d] Shutdown signal received. Exiting.\n", getpid());
            break;
        }
        sem_unlock(sem_stats);

        int idx = get_item_index(request.item);
        int price = get_item_price(request.item, &config);

        if (idx == -1) {
            printf("[Seller %d] Invalid item requested: %s\n", getpid(), request.item);
            continue;
        }

        int sold = 0;

        sem_lock(sem_final);
        if (final->final_products[idx] > 0) {
            final->final_products[idx]--;
            sold = 1;
            printf("[Seller %d] Sold %s to customer %d for $%d (Remaining: %d)\n",
                   getpid(), request.item, request.customer_pid, price, final->final_products[idx]);
        } else {
            // Out of stock → don't reply to customer
            printf("[Seller %d] Item out of stock: %s (Requested by %d)\n",
                   getpid(), request.item, request.customer_pid);
        }
        sem_unlock(sem_final);

        if (sold) {
            SellerReply reply;
            reply.mtype = request.customer_pid;
            reply.success = 1;
            reply.price = price;

            msgsnd(msqid, &reply, sizeof(reply) - sizeof(long), 0);

            sem_lock(sem_stats);
            stats->total_profit += price;
            printf("[Seller %d] Current total profit: $%d\n", getpid(), stats->total_profit);

            if ((rand() % 100) < config.COMPLAINT_PROBABILITY) {
                stats->complaints++;
                stats->total_profit -= price;
                printf("[Seller %d] Customer %d complained. Refunded $%d. Total complaints: %d. Profit now: $%d\n",
                       getpid(), request.customer_pid, price, stats->complaints, stats->total_profit);
            }

            if (stats->total_profit >= config.MAX_PROFIT ||
                stats->complaints >= config.MAX_CUSTOMER_COMPLAINTS ||
                stats->frustrated_customers >= config.MAX_FRUSTRATED_CUSTOMERS) {

                stats->shutdown_flag = 1;
                printf("[Seller %d] Shutdown condition met. Shutting down.\n", getpid());
            }
            sem_unlock(sem_stats);
        }
    }

    return 0;
}

