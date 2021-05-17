TARGET=mapgame
CC?=clang
CFLAGS=-lSDL2 -I./lib -Wall -Wextra -Wpedantic
CFILES=./src/*.c


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
