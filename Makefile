PHONY = all clean
CC = gcc
FLAGS = -Wall -Werror -g

all: myshell

myshell: myshell.o memory.o myshell.h
	$(CC) myshell.o memory.o -o myshell

myshell.o: myshell.c
	$(CC) $(FLAGS) -c $^

memory.o: memory.c
	$(CC) $(FLAGS) -c $^

clean:
	rm -f *.a *.o *.so *.gch myshell