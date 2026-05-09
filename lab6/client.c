#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "./include/common.h"
#include "./include/list.h"
#include "./include/utils.h"

// Max size of the datagrams that we will be sending
#define CHUNKSIZE MAXSIZE
#define SENT_FILENAME "client.c"
#define SERVER_IP "172.16.0.100"

#define TICK(X)                                                                \
  struct timespec X;                                                           \
  clock_gettime(CLOCK_MONOTONIC_RAW, &X)

#define TOCK(X)                                                                \
  struct timespec X##_end;                                                     \
  clock_gettime(CLOCK_MONOTONIC_RAW, &X##_end);                                \
  printf("Total time = %f seconds\n",                                          \
         (X##_end.tv_nsec - (X).tv_nsec) / 1000000000.0 +                      \
             (X##_end.tv_sec - (X).tv_sec))

list *window;

void send_file_start_stop(int sockfd, struct sockaddr_in server_address,
                          char *filename) {

  int fd = open(filename, O_RDONLY);
  DIE(fd < 0, "open");
  int rc;
  uint16_t seq = 1;

  char lastSentBuf[CHUNKSIZE];
  memset(lastSentBuf, 0, sizeof(lastSentBuf));

  uint16_t lastSentLen = 0;
  char resend = 0;

  while (1) {
    /* Reads a chunk of the file */
    struct seq_udp d;
    if (!resend) {
      int n = read(fd, d.payload, sizeof(d.payload));
      DIE(n < 0, "read");
      memcpy(lastSentBuf, d.payload, sizeof(lastSentBuf));
      lastSentLen = (uint16_t)n;
      d.len = htons((uint16_t)n);
      d.seq = htons(seq); // convert to network byte order
    } else {
      // resend the last packet
      memcpy(d.payload, lastSentBuf, sizeof(lastSentBuf));
      d.len = htons(lastSentLen);
      d.seq = htons(seq);
    }

    // Send the datagram
    rc = sendto(sockfd, &d, sizeof(struct seq_udp), 0,
                (struct sockaddr *)&server_address, sizeof(server_address));
    DIE(rc < 0, "send");

    // Wait for ACK before moving to the next datagram to send
    uint16_t ack[2];  // ack[0] as the ACK flag, ack[1] as the sequence number of the ACKed
    rc = recvfrom(sockfd, ack, sizeof(ack), 0, NULL, NULL);
    if (rc < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Timeout, resend the datagram
        resend = 1;
        continue;
      } else {
        // Other error on recvfrom
        DIE(rc < 0, "recv-ack");
      }
    } else if (ntohs(ack[0]) == 0 && ack[1] == d.seq) {
      // If we receive the ACK for the expected sequence number, we can move to the next datagram to send.
      // Clear the send buffer to see clearly the payload in wireshark
      memset(d.payload, 0, sizeof(d.payload));
      resend = 0;

      if (d.len == 0) {
        // end of file
        break;
      }

      seq++;
      continue;
    } else {
      // If we receive an ACK but it's not for the expected sequence number resend the datagram
      resend = 1;
      continue;
    }
  }
}

void send_window(int sockfd, struct sockaddr *serveraddr, socklen_t serveraddr_len, list *window) {
    list_entry* e = window->head;
    while(e != NULL && e->seq <= window->max_seq) {
      sendto(sockfd, e->info, e->info_len, 0, serveraddr, serveraddr_len);
      e = e->next;
    }
}

// seq = sequence number of the received ack
int update_left_window(list *window, int seq) {
    list_entry* e = window->head;
    int count = 0;

    while(e != NULL && e->seq <= seq) {
        list_entry *prev = e;
        e = e->next;
        free(prev->info);
        free(prev);
        count++;
    }

    window->head = e;
    return count;
}

void send_new_segments(int sockfd, struct sockaddr* serveraddr, socklen_t serveraddr_len, list *window, int new_max_seq) {
  list_entry* e = window->head;
  // Skip the segments that are already ACKed
  while(e != NULL && e->seq <= window->max_seq) {
    e = e->next;
  }

  // Send the new segments up to new_max_seq
  while(e != NULL && e->seq <= new_max_seq) {
    sendto(sockfd, e->info, e->info_len, 0, serveraddr, serveraddr_len);
    e = e->next;
  }

  window->max_seq = new_max_seq;
}

void send_file_go_back_n(int sockfd, struct sockaddr_in server_address,
                         char *filename) {

  int fd = open(filename, O_RDONLY);
  DIE(fd < 0, "open");
  int rc;

  // Window size: 10*10^6 b/s * 10*10^-3 s = 100000bits / 8 = BDP = 12500 bytes
  // => window_size = BDP / CHUNKSIZE = 12500 / 1024 = 12 (to also fit the inferior level headers)
  int window_size = 12;
  window->max_seq = window_size;  // Use 1-based indexing
  
  // Read the entire file in chunks and add them into a list of seq_udp (window)
  uint16_t seq = 1;
  while (1) {
    struct seq_udp *d = malloc(sizeof(struct seq_udp));
    DIE(d == NULL, "malloc");

    int n = read(fd, d->payload, sizeof(d->payload));
    DIE(n < 0, "read");
    d->len = htons((uint16_t)n);
    d->seq = htons(seq);

    add_list_elem(window, d, sizeof(struct seq_udp), seq);
    seq++;

    if (n == 0) // end of file
      break;
  }

  // Send window_size packets to the server to saturate the link
  send_window(sockfd, (struct sockaddr *)&server_address, sizeof(server_address), window);

  // In a loop, until the list of packets is empty
  while (window->head != NULL) {
    // On ACK remove from the list all the segments that have been ACKed
    // and send the next new segments added to the window
    uint16_t ack[2];
    rc = recvfrom(sockfd, ack, sizeof(ack), 0, NULL, NULL);
    if (rc < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // On timeout on recv resend all the segments from the window
        send_window(sockfd, (struct sockaddr *)&server_address, sizeof(server_address), window);
        continue;
      } else {
        // Other error on recvfrom
        DIE(rc < 0, "recv-ack");
      }
    } else if (ntohs(ack[0]) == 0) {
      uint16_t acked_seq = ntohs(ack[1]);
      if (window->head != NULL && acked_seq >= window->head->seq) {
        // Receive an ACK for a relevant segment => we can remove the ACKed segments from the window
        int removed_count = update_left_window(window, acked_seq);
        send_new_segments(sockfd, (struct sockaddr *)&server_address, sizeof(server_address), window, window->max_seq + removed_count);
      } else {
        // Receive an ACK but for a segment that is already ACKed => just make sure they are removed from the window
        update_left_window(window, acked_seq);
      }
    }
  }
}

void send_a_message(int sockfd, struct sockaddr_in server_address) {
  struct seq_udp d;
  strcpy(d.payload, "Hello world!");
  d.len = strlen("Hello world!");

  // Send a UDP datagram. Sendto is implemented in the kernel (network stack of
  // it), it basically creates a UDP datagram, sets the payload to the data we
  // specified in the buffer, and the completes the IP header and UDP header
  // using the sever_address info.
  int rc = sendto(sockfd, &d, sizeof(struct seq_udp), 0,
                  (struct sockaddr *)&server_address, sizeof(server_address));

  DIE(rc < 0, "send");

  // Receive the ACK. recvfrom is blocking with the current parameters 
  int ack;
  rc = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
}

int main(void) {

  // We use this structure to store the server info. IP address and Port.
  // This will be written by the UDP implementation on recvfrom().
  struct sockaddr_in servaddr;
  int sockfd, rc;

  // for benchmarking
  TICK(TIME_A);

  // Our transmission window
  window = create_list();

  // Creating socket file descriptor. SOCK_DGRAM for UDP
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(sockfd < 0, "socket");

  // Set the timeout on the socket
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 250000; // 250ms

  rc = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
  DIE(rc < 0, "setsockopt");

  // Fill the information that will be put into the IP and UDP header to
  // identify the target process (via PORT) on a given host (via SEVER_IP)
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  inet_aton(SERVER_IP, &servaddr.sin_addr);

  // send_a_message(sockfd, servaddr);
  // send_file_start_stop(sockfd, servaddr, SENT_FILENAME);
  send_file_go_back_n(sockfd, servaddr, SENT_FILENAME);

  close(sockfd);

  free(window);

  // Print the runtime of the program
  TOCK(TIME_A);

  return 0;
}
