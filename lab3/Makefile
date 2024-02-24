CFLAGS= -Wall -Werror -Wno-error=unused-variable -I../common/

all: send recv

../common/utils.o:
		$(MAKE) -C ../common

link_emulator/lib.o:
	$(MAKE) -C link_emulator

send: send.o link_emulator/lib.o common.o ../common/utils.o

recv: recv.o link_emulator/lib.o common.o ../common/utils.o

common.o: common.c common.h

clean:
	$(MAKE) -C link_emulator clean
	-rm -f *.o send recv common.h.gch

.PHONY: all clean
