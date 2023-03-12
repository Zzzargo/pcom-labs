We have **two separate programs**, a client and a server.

``client.c`` - the client implementation

``server.c`` - the server implementation

``queue.h`` - the queue API

``common.h`` - Some common data, such as the used port definition

Topology:
```
        L1          L2
client <--> router <--> server

Link 1 - 10 Mbps, 5ms delay, 0% packet loss (parameters set at topo.py:57)
Link 2 - 10 Mbps, 5ms delay, 0% packet loss (parameters set at topo.py:60)a


client IP (h1) - 192.168.1.100
server IP (h2) - 172.160.0.100
```
## API
We will use the `sockers API`. For the queue, we have a simple queue implementation
defined in `queue.h`.

```C
datagram_queue = queue_create();
queue datagram_queue;
seq_udp p;
queue_enq(datagram_queue, &p);

seq_udp *t;
t = queue_deq(datagram_queue)
```

Our protocol implementation will have the following datagram structure over UDP:

```C
struct seq_udp {    
  uint32_t len;    
  char payload[MAXSIZE];    
};
```

## Usage
To compile the code
```bash
make
# to create a random file of 1MB called file.bin
make random_file
```

This will create two binaries, to run them:
```bash
sudo python3 topo.py
```
It will open several terminals. We will run the client from `h1` and
the server from `h2`.

To run a benchmark, use:
```bash
make run_benchmark
```

## C++
Note, if you want to use C++, simply change the extension of `client.c` and
`server.c` to `.cpp` and update the Makefile to use `g++`. Further, you can
use `std::queue` as opposed to the queue implementation.
