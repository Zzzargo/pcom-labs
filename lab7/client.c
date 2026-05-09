/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP si mulplixare
 * client.c
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "helpers.h"

char username[64];

void run_client(int sockfd) {
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);

  struct chat_packet sent_packet;
  struct chat_packet recv_packet;

  /*
    Multiplexam intre citirea de la tastatura si primirea unui
    mesaj, ca sa nu mai fie impusa ordinea.
  */
  struct pollfd poll_fds[2];
  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[0].events = POLLIN;
  poll_fds[1].fd = sockfd;
  poll_fds[1].events = POLLIN;

  int rc;

  while (1) {
    rc = poll(poll_fds, 2, -1);
    DIE(rc < 0, "poll");

    if (poll_fds[0].revents & POLLIN) {
      // Am primit date de la tastatura
      fgets(buf + strlen(username) + 3, sizeof(buf) - strlen(username) - 3, stdin);
      if (isspace(buf[0])) {
        continue;
      }
      buf[0] = '[';
      buf[strlen(username) + 1] = ']';
      buf[strlen(username) + 2] = ' ';
      memcpy(buf + 1, username, strlen(username));
      sent_packet.len = strlen(buf);
      strcpy(sent_packet.message, buf);
      send_all(sockfd, &sent_packet, sizeof(sent_packet));
    }
    
    if (poll_fds[1].revents & POLLIN) {
      // Am primit un mesaj de la server
      rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
      if (rc <= 0) {
        printf("Conexiunea s-a inchis.\n");
        break;
      }
      printf("%s", recv_packet.message);  // \n e deja in mesaj de la tastatura
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("\n Usage: %s <ip> <port> <username>\n", argv[0]);
    return 1;
  }

  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[2], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  memset(username, 0, sizeof(username));
  memcpy(username, argv[3], strlen(argv[3]));

  // Obtinem un socket TCP pentru conectarea la server
  const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // Ne conectăm la server
  rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "connect");

  run_client(sockfd);

  // Inchidem conexiunea si socketul creat
  close(sockfd);

  return 0;
}
