#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

// Intellisense gets annoyed rather quickly with the includes
#include "../include/common.h"
#include "../include/queue.h"
#include "../include/utils.h"

#define TICK(X)                                                                \
  struct timespec X;                                                           \
  clock_gettime(CLOCK_MONOTONIC_RAW, &X)

#define TOCK(X)                                                                \
  struct timespec X##_end;                                                     \
  clock_gettime(CLOCK_MONOTONIC_RAW, &X##_end);                                \
  printf("Total time = %f seconds\n",                                          \
         (X##_end.tv_nsec - (X).tv_nsec) / 1000000000.0 +                      \
             (X##_end.tv_sec - (X).tv_sec))

/* Max size of the datagrams that we will be sending */
#define CHUNKSIZE 1024
#define SENT_FILENAME "README.md"
// #define SERVER_IP "172.16.0.100"
#define SERVER_IP "192.168.12.145"

/* Queue we will use for datagrams */
queue datagram_queue;

void send_file_start_stop(int sockfd, struct sockaddr_in server_address,
                          char *filename) {

  int fd = open(filename, O_RDONLY);
  DIE(fd < 0, "open");
  int rc;

  while (1) {
    /* Reads a chunk of the file */
    struct seq_udp d;
    int n = read(fd, d.payload, sizeof(d.payload));
    DIE(n < 0, "read");
    d.len = n;
    // Send the datagram
    rc = sendto(sockfd, &d, sizeof(struct seq_udp), 0,
                (struct sockaddr *)&server_address, sizeof(server_address));
    DIE(rc < 0, "send");

    // Wait for ACK before moving to the next datagram to send
    int ack;
    rc = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
    DIE(rc < 0, "recvfrom");
    DIE(ack != 0, "Invalid ACK received");
    
    if (n == 0) // end of file
      break;

  }
}

// It's important to note that the link is assumed to be perfect in this scenario
void send_file_window(int sockfd, struct sockaddr_in server_address,
                      char *filename) {
  int fd = open(filename, O_RDONLY);
  DIE(fd < 0, "open");
  int rc;

  // Window size = a value that optimally uses the link
  // Link1: 10MBps, 5ms delay, 0%loss
  // Link2: 10MBps, 5ms delay, 0%loss
  // => BDP = 10MBps * 5ms = 10^6 bytes/s * 5*10^-3 s = 5000 bytes
  // => window size = BDP / datagram size = 5000 bytes / 1024 bytes = ~4 datagrams
  // (inferior level protocol headers will have space)
  int window_size = 4;
  int packets_sent = 0;
  int acks_received = 0;
  int finished_reading = 0;

  while (!finished_reading || acks_received < packets_sent) {
    // Fill the window
    while (packets_sent - acks_received < window_size && !finished_reading) {
      struct seq_udp *d = malloc(sizeof(struct seq_udp));
      // Read a chunk of the file
      int n = read(fd, d->payload, sizeof(d->payload));
      DIE(n < 0, "read");
      
      if (n == 0) {
        finished_reading = 1;
        free(d);
        break;
      }

      d->len = n;
      // Send the chunk datagram right after reading it
      rc = sendto(sockfd, d, sizeof(struct seq_udp), 0,
                  (struct sockaddr *)&server_address, sizeof(server_address));
      DIE(rc < 0, "send");
      
      free(d);
      packets_sent++;
    }

    // If the window is full, wait for an ACK
    if (acks_received < packets_sent) {
      int ack;
      rc = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
      DIE(rc < 0, "recvfrom");
      DIE(ack != 0, "Invalid ACK received");
      acks_received++;
    }
  }

  // Finally send an empty datagram to signal the end of the file
  struct seq_udp d;
  d.len = 0;
  rc = sendto(sockfd, &d, sizeof(struct seq_udp), 0,
              (struct sockaddr *)&server_address, sizeof(server_address));
  DIE(rc < 0, "send");
}

void send_a_message(int sockfd, struct sockaddr_in server_address) {
  struct seq_udp d;
  strcpy(d.payload, "Hello world!");
  d.len = strlen("Hello world!");

  /* Send a UDP datagram. Sendto is implemented in the kernel (network stack of
   * it), it basically creates a UDP datagram, sets the payload to the data we
   * specified in the buffer, and the completes the IP header and UDP header
   * using the sever_address info.*/
  int rc = sendto(sockfd, &d, sizeof(struct seq_udp), 0,
                  (struct sockaddr *)&server_address, sizeof(server_address));

  DIE(rc < 0, "send");

  /* Receive the ACK. recvfrom is blocking with the current parameters */
  int ack;
  rc = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
}

int main(void) {

  /* We use this structure to store the server info. IP address and Port.
   * This will be written by the UDP implementation into the header */
  struct sockaddr_in servaddr;
  int sockfd, rc;

  // for benchmarking
  TICK(TIME_A);
  
  /* Queue that we will use when implementing sliding window */
  datagram_queue = queue_create();

  // Creating socket file descriptor. SOCK_DGRAM for UDP
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(sockfd < 0, "socket");

  // Fill the information that will be put into the IP and UDP header to
  // identify the target process (via PORT) on a given host (via SEVER_IP)
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  rc = inet_aton(SERVER_IP, &servaddr.sin_addr);
  DIE(rc == 0, "Invalid IP address for server");

  // Test (one at a time) each of the proposed versions for sending a file
  // send_a_message(sockfd, servaddr);
  // send_file_start_stop(sockfd, servaddr, SENT_FILENAME);
  send_file_window(sockfd, servaddr, SENT_FILENAME);

  /* Print the runtime of the program */
  TOCK(TIME_A);
  
  close(sockfd);

  return 0;
}
