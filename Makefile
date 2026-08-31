CC = gcc
CFLAGS = -Wall -Wextra -g `sdl-config --cflags`
LDFLAGS = `sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lm
SRC = main.c ennemi.c
OBJ = $(SRC:.c=.o)
EXEC = game

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

re: clean all






