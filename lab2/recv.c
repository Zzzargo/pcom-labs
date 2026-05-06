#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "include/utils.h"

/**
 * You can change these to communicate with another colleague.
 * There are several factors that could stop this from working over the
 * internet, but if you're on the same network it should work.
 * Just fill in their IP here and make sure that you use the same port.
 */
#define HOST "127.0.0.1"
#define PORT 10001

#include "common.h"

/* Our unique layer 2 ID */
static int ID = 123131;

/* Function which our protocol implementation will provide to the upper layer. */
int recv_frame(char *buf, int size)
{
	/* Call recv_byte() until we receive the frame start
	 * delimitator. This operation makes this function blocking until it
	 * receives a frame. */

	char c1, c2;
	c1 = recv_byte();
	c2 = recv_byte();

	while(((c1 != DLE) && (c2 != STX)) || (c1 == DLE && c2 != STX)
        || (c1 != DLE && c2 == STX)) {
		c1 = c2;
		c2 = recv_byte();
	}

	/* The first two 2 * sizeof(int) bytes represent sender and receiver ID */
	// Discard DLE STX as it's not needed data
	c1 = recv_byte();
	c2 = recv_byte();

	int dest;
	char addr[8];
	for (int i = 2; i < sizeof(addr); i++) {
		addr[i] = recv_byte();
	}
	dest = *(int *)(addr + 4);

	/* Check that the frame was sent to me */
	if (dest != ID)
		return 0;  // Not my precious

	// Get to reading the actual payload
	c1 = recv_byte();
	c2 = recv_byte();

	int i = 0;
	/* Read bytes and copy them to buff until we receive the end of the frame */
	while((((c1 != DLE) && (c2 != ETX)) || (c1 == DLE && c2 != ETX)
        || (c1 != DLE && c2 == ETX)) && i < size) {
		buf[i++] = c1;
		c1 = c2;
		c2 = recv_byte();
	}

	/* If everything went well return the number of bytes received */
	return i;
}

int main(int argc,char** argv){
	/* Don't modify this */
	init(HOST,PORT);

	/* Allocate a buffer and call recv_frame */
	char *buf = calloc(30, sizeof(char));
	recv_frame(buf, 30);
	printf("\n%s\n", buf);

	/* TODO 3: Measure latency in a while loop for any frame that contains
	 * a timestamp we receive, print frame_size and latency */

	printf("[RECEIVER] Finished transmission\n");
	return 0;
}
