#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define INGREDIENT_COUNT 8
#define PASTE_COUNT 1
#define BREAD_COUNT 1
#define READY_TO_BAKE_COUNT 5
#define FINAL_PRODUCT_COUNT 5
#define MAX_ITEM_NAME 32

// === Ingredient indices ===
enum {
    WHEAT,
    YEAST,
    BUTTER,
    MILK,
    SUGAR_SALT,
    SWEET_ITEMS,
    CHEESE,
    SALAMI
};

// === Paste index ===
enum {
    PASTE
};

// === Bread index ===
enum {
    BREAD
};

// === Ready-to-bake indices ===
enum {
    RAW_CAKE,
    RAW_SWEET,
    RAW_SWEET_PATISSERIE,
    RAW_SAVORY_PATISSERIE,
    RAW_BREAD
    
};

// === Final products indices ===
enum {
    SANDWICH,
    CAKE,
    SWEET,
    SWEET_PATISSERIE,
    SAVORY_PATISSERIE
};

// === Ingredient names ===
static const char* ingredient_names[] = {
    "Wheat", "Yeast", "Butter", "Milk", "Sugar/Salt", "Sweet Items", "Cheese", "Salami"
};

// === Paste names ===
static const char* paste_names[] = {
    "Paste"
};

// === Bread names ===
static const char* bread_names[] = {
    "Bread"
};

// === Ready-to-bake product names ===
static const char* ready_to_bake_names[] = {
    "Raw Cake", "Raw Sweet", "Raw Sweet Patisserie", "Raw Savory Patisserie", "Raw Bread"
};

// === Final product names ===
static const char* final_product_names[] = {
    "Sandwich", "Cake", "Sweet", "Sweet Patisserie", "Savory Patisserie"
};

// === Shared memory structures ===

// Ingredients (managed by suppliers)
typedef struct {
    int ingredients[INGREDIENT_COUNT];
} SharedBakery;

// Paste (produced by paste chefs, consumed by bakers and patisserie chefs)
typedef struct {
    int pastes[PASTE_COUNT];
} PasteStock;

// Bread (produced by bakers)
typedef struct {
    int breads[BREAD_COUNT];
} BreadStock;

// Ready-to-bake products (produced by chefs)
typedef struct {
    int ready_products[READY_TO_BAKE_COUNT];
} ReadyToBakeStock;

// Final customer-ready products (produced by bakers and sandwich chefs)
typedef struct {
    int final_products[FINAL_PRODUCT_COUNT];
} FinalProductStock;

typedef struct {
    int total_profit;
    int complaints;
    int frustrated_customers;
    time_t start_time; // used to track runtime
    int shutdown_flag; // 1 = signal to terminate
} SharedStats;


typedef struct {
    long mtype; // 1 for requests, customer PID for replies
    char item[MAX_ITEM_NAME];
    pid_t customer_pid;
} CustomerMessage;

typedef struct {
    long mtype;
    int success; // 1 = sold, 0 = out of stock or timeout
    int price;
} SellerReply;

// === IPC keys for shared memory and semaphores ===

#define SHM_KEY_INGREDIENTS  0x1234
#define SEM_KEY_INGREDIENTS  0x5678

#define SHM_KEY_PASTE        0x1324
#define SEM_KEY_PASTE        0x5768

#define SHM_KEY_BREAD        0x2345
#define SEM_KEY_BREAD        0x6789

#define SHM_KEY_READY        0x3456
#define SEM_KEY_READY        0x7890

#define SHM_KEY_FINAL        0x4567
#define SEM_KEY_FINAL        0x8901

#define STATS_SHM_KEY 0x9999
#define STATS_SEM_KEY 0x8888

#define MSG_KEY_CUSTOMER 0x1122



#endif
