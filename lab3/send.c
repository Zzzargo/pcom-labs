#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000


int main(int argc,char** argv) {
	init(HOST,PORT);

	/* Look in common.h for the definition of l3_msg */
	struct l3_msg t;

	/* We set the payload */
	sprintf(t.payload, "Hello World of PC");
	t.hdr.len = strlen(t.payload) + 1;

	/* Add the checksum */
	t.hdr.sum = simple_csum((void *) t.payload, t.hdr.len);

	/* TODO 2.0: Call crc32 function */

	/* Send the message */
	link_send(&t, sizeof(struct l3_msg));

	/* TODO 3.1: Receive the confirmation */

	/* TODO 3.2: If we received a NACK, retransmit the previous frame */

	/* TODO 3.3: Update this to read the content of a file and send it as
	 * chunks of that file given a MTU of 1500 bytes */

	return 0;
}
