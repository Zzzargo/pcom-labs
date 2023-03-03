We write the implementation of a forwarding process of a router.

``router.c`` - the code that will run on the router device

``lib.h`` - available API for this lab. Check the utilities function we have
defined there.

``protocols.h`` - structures for the protocols that we will be using today.
Ethernet and IP.

We have the following API today:

```c
/* Receives a packet. Returns the interface it has been received from.
Write to len the total size of the packet (how many bytes in buf are written) */
/* Blocking function, blocks if there is no packet to be received. */
int get_packet(char *packet, int *len);

/* Sends a packet on an interface. */
int send_packet(int interface, char *packet, int len);

/* Write to mac, a uint8_t mac[6] the MAC address of an interface */
int get_interface_mac(int interface, uint8_t *mac);

/* Returns the checksum of an IP header */
uint16_t ip_checksum(void* vdata, size_t length);
```

We use the following structures for the MAC table and route table:

```c
/* MAC Table Entry */
struct mac_entry {
	int32_t ip;
	uint8_t mac[6];
};

/* Route Table Entry */
struct rtable_entry {
	uint32_t network;
	uint32_t netmask;
	uint32_t nexthop;
	uint32_t metric;
	int interface;
};
```

## Topology


## Usage

To compile the code
```
make
```

To start the topology:
```bash
sudo pkill ovs-test
sudo python3 topo.py
```

To start the `router`, in the `router` device, simply run `./router`.

## C++

Note, if you want to use C++, simply change the extension of the `router.cpp` 
and update the Makefile to use `g++`.