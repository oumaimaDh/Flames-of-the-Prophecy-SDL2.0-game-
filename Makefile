CC = gcc
CFLAGS = -Wall
LDFLAGS = -lSDL -lSDL_image -lSDL_gfx

SRC = main.c
EXEC = mygamesss

all:
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(EXEC)











