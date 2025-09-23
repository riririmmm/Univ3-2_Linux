CC = gcc
MY_INC = ./include

multy_exe: main.o copy.o
	gcc -o multy_exe main.o copy.o

main.o: main.c
	gcc -c main.c -I ./include

copy.o: copy.c
	gcc -c copy.c -I ./include


