all: mymalloc.o memgrind.c
	gcc memgrind.c -o memgrind mymalloc.o
mymalloc.o: mymalloc.c
	gcc -c mymalloc.c
clean:
	rm memgrind; rm mymalloc.o
