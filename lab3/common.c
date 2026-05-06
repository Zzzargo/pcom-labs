#include "common.h"

uint8_t simple_csum(uint8_t *buf, size_t len) {
	uint8_t sum = 0;

	for (int i = 0; i < len; i++) {
		sum += buf[i];
	}
	return sum;
}

uint32_t crc32(uint8_t *buf, size_t len)
{
    uint32_t crc = ~0; // 0xffffffff
    const uint32_t POLY = 0xEDB88320;  // CRC32 polynomial

	// Iterate through each byte of buff
    while(len--) {
        crc = crc ^ *buf++;
		/* Iterate through each bit */
		/* If the bit is 1, compute the new reminder */
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ POLY;
            else 
                crc = (crc >> 1);
        }
    }

	/* By convention, we negate the crc */
    crc = ~crc;
    return crc;
}
