#------------------------------------------
#@author	: Sniehovskyi Nikita, xsnieh00
#			: FIT VUTBR.CZ
#@date		: 23.04.2022
#project	: IOS-Project-2
#@file		: makefile
#compiler	: -
#------------------------------------------


CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -D_XOPEN_SOURCE=700 -pthread -lrt #-Werror
EXEC = proj2

all: $(EXEC)

#----------- proj2

proj2: proj2.o error.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: src/%.c
	$(CC) -c $(CFLAGS) $^ -o $@





test:
	@echo "TODO"	

zip:
	zip "proj2.zip" modules/*/* src/*.c src/*.h makefile libs

clear:
	rm objs/*.o $(EXEC) libs/*.a