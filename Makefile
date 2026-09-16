CC = gcc

femto: src/femto.c
	$(CC) src/femto.c -o bin/femto -Wall -Wextra
