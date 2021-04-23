TARGET=mapgame
CC?=clang
CFLAGS=-lSDL2 -I./lib -Wall -Wextra -Wpedantic

CFILES=./src/main.c ./src/rand.c ./src/maps.c ./src/disp.c ./src/play.c ./src/loot.c ./src/font.c ./src/maus.c ./src/devm.c ./src/harv.c ./src/hold.c ./src/make.c ./src/save.c ./src/home.c

default:
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES)
clean:
	rm -f $(TARGET)
debug:
	$(CC) $(CFLAGS) -o $(TARGET) -g $(CFILES)
	#valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./mapgame
run: default
	./$(TARGET)
all: default
