#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

extern "C" void sha256_block_data_order (uint32_t *ctx, const void *in, size_t num);

// SHA-256 initial hash value
const uint32_t H_0[8] = {
	0x6a09e667,
	0xbb67ae85,
	0x3c6ef372,
	0xa54ff53a,
	0x510e527f,
	0x9b05688c,
	0x1f83d9ab,
	0x5be0cd19,
};


void sha256_print_hash(uint32_t *H, const char *title) {
	printf("-----------------------------------------------------------------------------------------\n");
	printf("| %-85s |\n", title);
	printf("-----------------------------------------------------------------------------------------\n");
	printf("|   H[0]   |   H[1]   |   H[2]   |   H[3]   |   H[4]   |   H[5]   |   H[6]   |   H[7]   |\n");
	printf("| %08x | %08x | %08x | %08x | %08x | %08x | %08x | %08x |\n", H[0], H[1], H[2], H[3], H[4], H[5], H[6], H[7]);
	printf("-----------------------------------------------------------------------------------------\n");
}


// initialize hash value
void sha256_init(uint32_t *H) {
	H[0] = 0x6a09e667;
	H[1] = 0xbb67ae85;
	H[2] = 0x3c6ef372;
	H[3] = 0xa54ff53a;
	H[4] = 0x510e527f;
	H[5] = 0x9b05688c;
	H[6] = 0x1f83d9ab;
	H[7] = 0x5be0cd19;
}


void FastMine(unsigned char* input, size_t len, unsigned char* output) {
	unsigned int i;

    unsigned char temp[512];
    memcpy(temp, input, len);
	// initialize hash value
	uint32_t H[8];
	memcpy(H, H_0, 8*4);

    uint64_t bits = len * 8;;
	unsigned char* buffer;
    
	
    for (buffer = temp; strlen((const char*) buffer) >= 64;  buffer += 64) {
        sha256_block_data_order(H, buffer,1);
        len -= 64;
	}

	// add padding
	if (len < 56) {
		// padd current block to 56 byte
		buffer[len] = 0x80;
		i = len + 1;
	} else {
		// fill up current block and update hash
		buffer[len] = 0x80;
		for (i = len + 1; i < 64; i++) {
			buffer[i] = 0x00;
		}

		sha256_block_data_order(H, buffer,1);

		// add (almost) one block of zero bytes
		i = 0;
	}
	for (; i < 56; i++) {
		buffer[i] = 0x00;
	}

	// add message length in bits in big endian
	for (i = 0; i < 8; i++) {
		buffer[63 - i] = bits >> (i * 8);
	}

	sha256_block_data_order(H, buffer,1);

	// print hash
	//sha256_print_hash(H, "Final Hash");

	// convert hash to char array (in correct order)
	for (i = 0; i < 8; i++) {
		output[i*4 + 0] = H[i] >> 24;
		output[i*4 + 1] = H[i] >> 16;
		output[i*4 + 2] = H[i] >>  8;
		output[i*4 + 3] = H[i];
	}
}

