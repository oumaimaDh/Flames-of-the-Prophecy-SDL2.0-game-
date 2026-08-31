# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_image -lSDL_mixer -lSDL_ttf -lm

# Sources and output
SRC = main.c source.c
OBJ = $(SRC:.c=.o)
EXEC = game

# Default target
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

re: clean all

