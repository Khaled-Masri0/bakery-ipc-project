#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "config_parser.h"
#include "common.h"

typedef enum {
    TEAM_PASTE,
    TEAM_RAW_BREAD,
    TEAM_CAKE,
    TEAM_SWEET,
    TEAM_SWEET_PATISSERIE,
    TEAM_SAVORY_PATISSERIE,
    TEAM_SANDWICH,
} ChefTeam;

ChefTeam get_team_type(const char* name) {
    if (strcmp(name, "paste") == 0) return TEAM_PASTE;
    if (strcmp(name, "rawbread") == 0) return TEAM_RAW_BREAD;
    if (strcmp(name, "cake") == 0) return TEAM_CAKE;
    if (strcmp(name, "sweet") == 0) return TEAM_SWEET;
    if (strcmp(name, "sweetp") == 0) return TEAM_SWEET_PATISSERIE;
    if (strcmp(name, "savoryp") == 0) return TEAM_SAVORY_PATISSERIE;
    if (strcmp(name, "sandwich") == 0) return TEAM_SANDWICH;
    return 0;
}

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void random_sleep(int min_sec, int max_sec) {
    sleep(min_sec + rand() % (max_sec - min_sec + 1));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [paste|rawbread|cake|sweet|sweetp|savoryp|sandwich]\n", argv[0]);
        return 1;
    }

    

    srand(time(NULL) ^ getpid());
    printf("[Chef %d] Started as team: %s\n", getpid(), argv[1]);
    ChefTeam team = get_team_type(argv[1]);

    // Attach to shared memories
    int shm_ing = shmget(SHM_KEY_INGREDIENTS, sizeof(SharedBakery), 0666);
    SharedBakery* ingredients = (SharedBakery*) shmat(shm_ing, NULL, 0);
    int sem_ing = semget(SEM_KEY_INGREDIENTS, 1, 0666);

    int shm_paste = shmget(SHM_KEY_PASTE, sizeof(PasteStock), 0666);
    PasteStock* paste = (PasteStock*) shmat(shm_paste, NULL, 0);
    int sem_paste = semget(SEM_KEY_PASTE, 1, 0666);

    int shm_bread = shmget(SHM_KEY_BREAD, sizeof(BreadStock), 0666);
    BreadStock* bread = (BreadStock*) shmat(shm_bread, NULL, 0);
    int sem_bread = semget(SEM_KEY_BREAD, 1, 0666);

    int shm_ready = shmget(SHM_KEY_READY, sizeof(ReadyToBakeStock), 0666);
    ReadyToBakeStock* ready = (ReadyToBakeStock*) shmat(shm_ready, NULL, 0);
    int sem_ready = semget(SEM_KEY_READY, 1, 0666);

    int shm_final = shmget(SHM_KEY_FINAL, sizeof(FinalProductStock), 0666);
    FinalProductStock* final = (FinalProductStock*) shmat(shm_final, NULL, 0);
    int sem_final = semget(SEM_KEY_FINAL, 1, 0666);
    
    int shm_stats = shmget(STATS_SHM_KEY, sizeof(SharedStats), 0666);
    if (shm_stats == -1) { perror("shmget stats"); exit(1); }

   SharedStats* stats = (SharedStats*) shmat(shm_stats, NULL, 0);
    if (stats == (void*) -1) { perror("shmat stats"); exit(1); }

    int sem_stats = semget(STATS_SEM_KEY, 1, 0666);  
    if (sem_stats == -1) { perror("semget stats"); exit(1); }

    
    Config config; 
  if (load_config("config/config.txt", &config) != 0) {
    fprintf(stderr, "[Chef %d] Failed to load config.\n", getpid());
    exit(1);
}


    while (1) {
    
    sem_lock(sem_stats);
 if (stats->shutdown_flag) {
    sem_unlock(sem_stats);
    printf("[Chef %d] Shutdown detected. Exiting.\n", getpid());
    break;
 }
 sem_unlock(sem_stats);

        if (team == TEAM_PASTE) {
           sem_lock(sem_ing);
if (ingredients->ingredients[WHEAT] > 0 &&
    ingredients->ingredients[YEAST] > 0 &&
    ingredients->ingredients[MILK] > 0) {

    ingredients->ingredients[WHEAT]--;
    ingredients->ingredients[YEAST]--;
    ingredients->ingredients[MILK]--;
    sem_unlock(sem_ing);

    sem_lock(sem_paste);
    if (paste->pastes[PASTE] < config.MAX_PASTE) {
        paste->pastes[PASTE]++;
        printf("[Chef %d] Produced Paste. Total: %d\n", getpid(), paste->pastes[PASTE]);
    } else {
        printf("[Chef %d] Max Paste limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_paste);

} else {
    printf("[Chef %d] Not enough ingredients for Paste.\n", getpid());
    sem_unlock(sem_ing);
}

        }

        else if (team == TEAM_RAW_BREAD) {
            sem_lock(sem_ing);
   if (ingredients->ingredients[WHEAT] > 0 &&
      ingredients->ingredients[YEAST] > 0 &&
      ingredients->ingredients[MILK] > 0) {

      ingredients->ingredients[WHEAT]--;
      ingredients->ingredients[YEAST]--;
      ingredients->ingredients[MILK]--;
      sem_unlock(sem_ing);

    sem_lock(sem_ready);
    if (ready->ready_products[RAW_BREAD] < config.MAX_RAW_BREAD) {
        ready->ready_products[RAW_BREAD]++;
        printf("[Chef %d] Created Raw Bread. Total: %d\n", getpid(), ready->ready_products[RAW_BREAD]);
    } else {
        printf("[Chef %d] Max Raw Bread limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_ready);
}
 else {
    printf("[Chef %d] Not enough ingredients for Raw Bread.\n", getpid());
    sem_unlock(sem_ing);
}

        }

        else if (team == TEAM_CAKE) {
            sem_lock(sem_ing);
if (ingredients->ingredients[WHEAT] > 0 &&
    ingredients->ingredients[BUTTER] > 0 &&
    ingredients->ingredients[MILK] > 0 &&
    ingredients->ingredients[SUGAR_SALT] > 0) {

    ingredients->ingredients[WHEAT]--;
    ingredients->ingredients[BUTTER]--;
    ingredients->ingredients[MILK]--;
    ingredients->ingredients[SUGAR_SALT]--;
    sem_unlock(sem_ing);

    sem_lock(sem_ready);
    if (ready->ready_products[RAW_CAKE] < config.MAX_RAW_CAKE) {
        ready->ready_products[RAW_CAKE]++;
        printf("[Chef %d] Created Raw Cake. Total: %d\n", getpid(), ready->ready_products[RAW_CAKE]);
    } else {
        printf("[Chef %d] Max Raw Cake limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_ready);
} else {
    printf("[Chef %d] Not enough ingredients for Cake.\n", getpid());
    sem_unlock(sem_ing);
}

        }

        else if (team == TEAM_SWEET) {
            sem_lock(sem_ing);
if (ingredients->ingredients[SWEET_ITEMS] > 0 &&
    ingredients->ingredients[SUGAR_SALT] > 0) {

    ingredients->ingredients[SWEET_ITEMS]--;
    ingredients->ingredients[SUGAR_SALT]--;
    sem_unlock(sem_ing);

    sem_lock(sem_ready);
    if (ready->ready_products[RAW_SWEET] < config.MAX_RAW_SWEET) {
        ready->ready_products[RAW_SWEET]++;
        printf("[Chef %d] Created Raw Sweet. Total: %d\n", getpid(), ready->ready_products[RAW_SWEET]);
    } else {
        printf("[Chef %d] Max Raw Sweet limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_ready);
} else {
    printf("[Chef %d] Not enough ingredients for Sweet.\n", getpid());
    sem_unlock(sem_ing);
}

        }

        else if (team == TEAM_SWEET_PATISSERIE) {
           sem_lock(sem_paste);
sem_lock(sem_ing);
if (paste->pastes[PASTE] > 0 &&
    ingredients->ingredients[SWEET_ITEMS] > 0) {

    paste->pastes[PASTE]--;
    ingredients->ingredients[SWEET_ITEMS]--;
    sem_unlock(sem_ing);
    sem_unlock(sem_paste);

    sem_lock(sem_ready);
    if (ready->ready_products[RAW_SWEET_PATISSERIE] < config.MAX_RAW_SWEET_PATISSERIE) {
        ready->ready_products[RAW_SWEET_PATISSERIE]++;
        printf("[Chef %d] Created Raw Sweet Patisserie. Total: %d\n", getpid(), ready->ready_products[RAW_SWEET_PATISSERIE]);
    } else {
        printf("[Chef %d] Max Sweet Patisserie limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_ready);
} else {
    printf("[Chef %d] Not enough paste or sweet items.\n", getpid());
    sem_unlock(sem_ing);
    sem_unlock(sem_paste);
}

        }

        else if (team == TEAM_SAVORY_PATISSERIE) {
            sem_lock(sem_paste);
sem_lock(sem_ing);
if (paste->pastes[PASTE] > 0 &&
    ingredients->ingredients[CHEESE] > 0) {

    paste->pastes[PASTE]--;
    ingredients->ingredients[CHEESE]--;
    sem_unlock(sem_ing);
    sem_unlock(sem_paste);

    sem_lock(sem_ready);
    if (ready->ready_products[RAW_SAVORY_PATISSERIE] < config.MAX_RAW_SAVORY_PATISSERIE) {
        ready->ready_products[RAW_SAVORY_PATISSERIE]++;
        printf("[Chef %d] Created Raw Savory Patisserie. Total: %d\n", getpid(), ready->ready_products[RAW_SAVORY_PATISSERIE]);
    } else {
        printf("[Chef %d] Max Savory Patisserie limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_ready);
} else {
    printf("[Chef %d] Not enough paste or cheese.\n", getpid());
    sem_unlock(sem_ing);
    sem_unlock(sem_paste);
}

        }

        else if (team == TEAM_SANDWICH) {
            sem_lock(sem_bread);
sem_lock(sem_ing);
if (bread->breads[BREAD] > 0 &&
    ingredients->ingredients[CHEESE] > 0 &&
    ingredients->ingredients[SALAMI] > 0) {

    bread->breads[BREAD]--;
    ingredients->ingredients[CHEESE]--;
    ingredients->ingredients[SALAMI]--;
    sem_unlock(sem_ing);
    sem_unlock(sem_bread);

    sem_lock(sem_final);
    if (final->final_products[SANDWICH] < config.MAX_SANDWICH) {
        final->final_products[SANDWICH]++;
        printf("[Chef %d] Prepared Sandwich. Total: %d\n", getpid(), final->final_products[SANDWICH]);
    } else {
        printf("[Chef %d] Max Sandwich limit reached. Skipping.\n", getpid());
    }
    sem_unlock(sem_final);
} else {
    printf("[Chef %d] Not enough ingredients or bread for Sandwich.\n", getpid());
    sem_unlock(sem_ing);
    sem_unlock(sem_bread);
}

        }

        random_sleep(2, 5);
    }

    return 0;
}
