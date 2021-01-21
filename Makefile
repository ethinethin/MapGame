TARGET=mapgame
CC?=clang
CFLAGS=-lSDL2 -I./lib -Wall -Wextra -Wpedantic

CFILES=./src/main.c ./src/rand.c ./src/maps.c ./src/disp.c ./src/play.c

default:
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES)
clean:
	rm -f $(TARGET)
run: default
	./$(TARGET)
all: default
