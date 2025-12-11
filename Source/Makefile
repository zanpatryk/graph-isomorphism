# Compiler and Flags
CC = gcc
CFLAGS = -std=c11 -O3 -I.
# Target Executable Name
TARGET = aac

# Source Files
SRCS = main.c \
       utils.c \
       graph.c \
       algorithms/minimal_extension.c \
       algorithms/minimal_extension_approximation.c \
       algorithms/product_graph.c \
       algorithms/isomorphism.c

# Object Files
OBJS = $(SRCS:.c=.o)

# Default Rule
all: $(TARGET)

# Link Rule: Creates the executable AND deletes object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Linking complete. Removing object files..."
	rm -f $(OBJS)

# Compilation Rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Rule (Only needs to remove the executable now)
clean:
	rm -f $(TARGET)

.PHONY: all clean