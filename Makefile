# Defining compiler
CC = gcc

# Defining flags
CFLAGS = -std=c99 -Wall

# Defining target executable
TARGET = memory_allocation_simulation

# Defining source file
SOURCES = memory_allocation_simulation.c functions.c

# Defining object file
OBJECTS = $(SOURCES:.c=.o)

# Defineing header files
HEADERS = functions.h

# Compile program from target
all: $(TARGET) run
# Compile source file into the target executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source file into object file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run program
run: $(TARGET)
	./$(TARGET)
	make clean

# Clean directory
clean: 
	rm -f $(TARGET) $(OBJECTS)