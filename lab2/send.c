#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"
#include "include/utils.h"

/**
 * You can change these to communicate with another colleague.
 * There are several factors that could stop this from working over the
 * internet, but if you're on the same network it should work.
 * Just fill in their IP here and make sure that you use the same port.
 */
#define HOST "127.0.0.1"
#define PORT 10000

/* Here we have the Frame structure */
#include "common.h"

/* Our unqiue layer 2 ID */
static int ID = 123131;

/* Function which our protocol implementation will provide to the upper layer. */
int send_frame(char *buf, int size)
{

	/* Create a new frame. */
	struct Frame *frame = calloc(1, sizeof(struct Frame));
	frame->frame_delim_start[0] = DLE;
	frame->frame_delim_start[1] = STX;
	frame->frame_delim_end[0] = DLE;
	frame->frame_delim_end[1] = ETX;

	/* Copy the data from buffer to our frame structure */
	memcpy(frame->payload, buf, size);

	/* Set the destination and source */
	frame->source = ID;
	frame->dest = 123131;

	/* We can cast the frame to a char *, and iterate through sizeof(struct Frame) bytes calling send_bytes. */
	uint8_t *byteFrame = (uint8_t *)frame;
	int frameRawSize = sizeof(struct Frame);

	for (int i = 0; i < frameRawSize; i++) {
		send_byte(byteFrame[i]);
	}

	/* if all went all right, return 0 */
	return 0;
}

int main(int argc,char** argv){
	// Don't touch this
	init(HOST,PORT);

	/* Get some input in a buffer and call send_frame with it */
	char buf[30] = "Sa-mi bag pula waaai petrea";
	send_frame(buf, sizeof(buf) / sizeof(buf[0]));

	/* TODO 3.1: Get a timestamp of the current time copy it in the the payload */

	/* TODO 3.0: Update the maximum size of the payload in Frame to 100 (in common.h), send the frame */

	/* TODO 3.0: Update the maximum size of the payload in Frame to 300, send the frame */

	return 0;
}
