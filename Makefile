CC = gcc
CFLAGS = -Wall -Wextra -g -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lm
OBJECTS = main.o source.o
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

source.o: source.c
	$(CC) -c source.c -o source.o $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
