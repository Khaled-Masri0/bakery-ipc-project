#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_parser.h"

int load_config(const char* filename, Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open config file");
        return -1;
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char key[64];
        int value;
        if (sscanf(line, "%[^=]=%d", key, &value) == 2) {

            // Supplier and chef team counts
            if (strcmp(key, "NUM_SUPPLIERS") == 0) config->NUM_SUPPLIERS = value;

            else if (strcmp(key, "NUM_CHEFS_PASTE") == 0) config->NUM_CHEFS_PASTE = value;
            else if (strcmp(key, "NUM_CHEFS_CAKES") == 0) config->NUM_CHEFS_CAKES = value;
            else if (strcmp(key, "NUM_CHEFS_SANDWICHES") == 0) config->NUM_CHEFS_SANDWICHES = value;
            else if (strcmp(key, "NUM_CHEFS_SWEETS") == 0) config->NUM_CHEFS_SWEETS = value;
            else if (strcmp(key, "NUM_CHEFS_SWEET_PATISSERIES") == 0) config->NUM_CHEFS_SWEET_PATISSERIES = value;
            else if (strcmp(key, "NUM_CHEFS_SAVORY_PATISSERIES") == 0) config->NUM_CHEFS_SAVORY_PATISSERIES = value;
            else if (strcmp(key, "NUM_CHEFS_RAWBREAD") == 0) config->NUM_CHEFS_RAWBREAD = value;
            
            
            else if(strcmp(key, "NUM_BAKERS_CAKESWEETS") == 0) config->NUM_BAKERS_CAKESWEETS = value;
            else if(strcmp(key, "NUM_BAKERS_PATISSERIES") == 0) config->NUM_BAKERS_PATISSERIES = value;
            else if(strcmp(key, "NUM_BAKERS_BREAD") == 0) config->NUM_BAKERS_BREAD = value;
            
            

            // Ingredient thresholds
            else if (strcmp(key, "WHEAT_MIN") == 0) config->WHEAT_MIN = value;
            else if (strcmp(key, "WHEAT_MAX") == 0) config->WHEAT_MAX = value;

            else if (strcmp(key, "YEAST_MIN") == 0) config->YEAST_MIN = value;
            else if (strcmp(key, "YEAST_MAX") == 0) config->YEAST_MAX = value;

            else if (strcmp(key, "BUTTER_MIN") == 0) config->BUTTER_MIN = value;
            else if (strcmp(key, "BUTTER_MAX") == 0) config->BUTTER_MAX = value;

            else if (strcmp(key, "MILK_MIN") == 0) config->MILK_MIN = value;
            else if (strcmp(key, "MILK_MAX") == 0) config->MILK_MAX = value;

            else if (strcmp(key, "SUGAR_SALT_MIN") == 0) config->SUGAR_SALT_MIN = value;
            else if (strcmp(key, "SUGAR_SALT_MAX") == 0) config->SUGAR_SALT_MAX = value;

            else if (strcmp(key, "SWEET_ITEMS_MIN") == 0) config->SWEET_ITEMS_MIN = value;
            else if (strcmp(key, "SWEET_ITEMS_MAX") == 0) config->SWEET_ITEMS_MAX = value;

            else if (strcmp(key, "CHEESE_SALAMI_MIN") == 0) config->CHEESE_SALAMI_MIN = value;
            else if (strcmp(key, "CHEESE_SALAMI_MAX") == 0) config->CHEESE_SALAMI_MAX = value;

           
            else if (strcmp(key, "MAX_RAW_BREAD") == 0) config->MAX_RAW_BREAD = value;
            else if (strcmp(key, "MAX_RAW_CAKE") == 0) config->MAX_RAW_CAKE = value;
            else if (strcmp(key, "MAX_RAW_SWEET") == 0) config->MAX_RAW_SWEET = value;
            else if (strcmp(key, "MAX_RAW_SWEET_PATISSERIE") == 0) config->MAX_RAW_SWEET_PATISSERIE = value;
            else if (strcmp(key, "MAX_RAW_SAVORY_PATISSERIE") == 0) config->MAX_RAW_SAVORY_PATISSERIE = value;

            
            else if (strcmp(key, "MAX_BREAD") == 0) config->MAX_BREAD = value;
            else if (strcmp(key, "MAX_CAKE") == 0) config->MAX_CAKE = value;
            else if (strcmp(key, "MAX_SWEET") == 0) config->MAX_SWEET = value;
            else if (strcmp(key, "MAX_SWEET_PATISSERIE") == 0) config->MAX_SWEET_PATISSERIE = value;
            else if (strcmp(key, "MAX_SAVORY_PATISSERIE") == 0) config->MAX_SAVORY_PATISSERIE = value;
            else if (strcmp(key, "MAX_SANDWICH") == 0) config->MAX_SANDWICH = value;
            else if (strcmp(key, "MAX_PASTE") == 0) config->MAX_PASTE = value;

            else if (strcmp(key, "PRICE_SANDWICH") == 0) config->PRICE_SANDWICH = value;
            else if (strcmp(key, "PRICE_CAKE") == 0) config->PRICE_CAKE = value;
            else if (strcmp(key, "PRICE_SWEET") == 0) config->PRICE_SWEET = value;
            else if (strcmp(key, "PRICE_SWEET_PATISSERIE") == 0) config->PRICE_SWEET_PATISSERIE = value;
            else if (strcmp(key, "PRICE_SAVORY_PATISSERIE") == 0) config->PRICE_SAVORY_PATISSERIE = value;

            else if (strcmp(key, "MAX_FRUSTRATED_CUSTOMERS") == 0) config->MAX_FRUSTRATED_CUSTOMERS = value;
            else if (strcmp(key, "MAX_CUSTOMER_COMPLAINTS") == 0) config->MAX_CUSTOMER_COMPLAINTS = value;
     
            else if (strcmp(key, "MAX_PROFIT") == 0) config->MAX_PROFIT = value;
            else if (strcmp(key, "MAX_RUNTIME_MINUTES") == 0) config->MAX_RUNTIME_MINUTES = value;

            else if (strcmp(key, "NUM_SELLERS") == 0) config->NUM_SELLERS = value;
       
            else if (strcmp(key, "COMPLAINT_PROBABILITY") == 0) config->COMPLAINT_PROBABILITY = value;
            else if (strcmp(key, "CUSTOMER_WAIT_TIME") == 0) config->CUSTOMER_WAIT_TIME = value;


        }
    }

    fclose(file);
    return 0;
}
