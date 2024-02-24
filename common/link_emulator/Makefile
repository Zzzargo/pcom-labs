all: link lib.o

link: link.o queue.o
	gcc -g link.o queue.o -o link -pthread

.c.o:
	gcc -Wall -g -c $? -pthread -I../

clean:
	rm -f *.o link
