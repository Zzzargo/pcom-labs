#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdint.h>

#define MAXSIZE 1024
#define PORT 8313

struct seq_udp {
  uint32_t len;
  char payload[MAXSIZE];
};

#endif
