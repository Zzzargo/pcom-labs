CFLAGS= -Wall -g -pthread -I../ -MMD -MP

all: link lib.o

link: link.o queue.o
	gcc -g link.o queue.o -o link -pthread

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

# Include auto-generated dependency files (silently ignored on first build)
-include $(wildcard *.d)

clean:
	rm -f *.o *.d link
