# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Directories
SRC = src
INCLUDE = include

# Object files
OBJ_MAIN = $(SRC)/main.o $(SRC)/config_parser.o
OBJ_SUPPLIER = $(SRC)/supplier.o $(SRC)/config_parser.o
OBJ_CHEF = $(SRC)/chef.o $(SRC)/config_parser.o
OBJ_BAKER = $(SRC)/baker.o $(SRC)/config_parser.o
OBJ_CUSTOMER = $(SRC)/customer.o $(SRC)/config_parser.o
OBJ_SELLER = $(SRC)/seller.o $(SRC)/config_parser.o


# Targets
all: main supplier chef baker customer seller

# Build main controller
main: $(OBJ_MAIN)
	$(CC) $(CFLAGS) -o main $(OBJ_MAIN)

# Build supplier program
supplier: $(OBJ_SUPPLIER)
	$(CC) $(CFLAGS) -o supplier $(OBJ_SUPPLIER)

# Build chef program
chef: $(OBJ_CHEF)
	$(CC) $(CFLAGS) -o chef $(OBJ_CHEF)

# Build baker program
baker: $(OBJ_BAKER)
	$(CC) $(CFLAGS) -o baker $(OBJ_BAKER)

# Build customer program
customer: $(OBJ_CUSTOMER)
	$(CC) $(CFLAGS) -o customer $(OBJ_CUSTOMER)
	
# Build seller program
seller: $(OBJ_SELLER)
	$(CC) $(CFLAGS) -o seller $(OBJ_SELLER)


# Compile C files into object files
$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up everything
clean:
	rm -f $(SRC)/*.o main supplier chef baker customer seller
