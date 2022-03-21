#include <iostream>
#include <time.h>
#include <random>

#include "lib/sha256.c"
#include "lib/to_text_from_integer.h"
#include "TFutils.hpp"

#define NUM_THREADS 8


using namespace std;

uint64_t parallel_mine(const string& constant, const string& target) {
    // initialize the thread pool
    vector<thread> threads;
    threads.reserve(NUM_THREADS);

    uint64_t result;
    bool is_odd = target.length() % 2, done = false;
    vector<unsigned char> binary_target;
    unsigned char odd = 0; // stores the odd half-byte part of the target

    // convert the target to binary form
    if (is_odd) {
        binary_target = hex2bin(target.substr(0, target.length() - 1));
        odd = (target.back() >= 'a') ? (target.back() - 'a' + 10) : (target.back() - '0');
        odd = odd << 4; // left shift so that it's stored in the first half of the byte
    } else {
        binary_target = hex2bin(target);
    }

    // generate a random start nonce to prevent duplicate forks
    random_device rd;
    default_random_engine generator(rd());
    uniform_int_distribution<long long unsigned> distribution(0,0xFFFFFFFFFFFFFFFF);
    uint64_t start_nonce = 0;

    // calculate and store the SHA context after transforming the first 2 64 byte chunks (source-hash and data-hash)
    uint32_t H_1[8];
    memcpy(H_1, H_0, 32);
    unsigned int len = constant.length();

    for (const char *chunk = constant.c_str(); len >= 64; chunk += 64) {
       //printf("chunk: %.64s\n", chunk);
        sha256_block_data_order(H_1, chunk,1);
        len -= 64;
    }

    // for (int i = 0; i < 8; i++) {
    //     printf("H_1[%d]: %u\n", i, H_1[i]);
    // }

    // store the leftover data for the final chunk which will be transformed every iteration
    const string leftover = constant.substr(constant.length() - len, len);

    //std::cout << "leftover: " << leftover << endl;

    for (int j = 0; j < NUM_THREADS; j++) {
        threads.emplace_back(thread([&result, &done, j, binary_target, is_odd, odd, start_nonce, leftover, H_1, constant]{
            unsigned char digest[32], buffer[64] = {0};
            uint32_t H[8];

            // copy the user and target into the buffer
            memcpy(buffer, leftover.c_str(), leftover.length());

            // calculate the start nonce for this thread
            uint64_t nonce = 1000000000000;

            // points to the place in the buffer where the nonce is to be stored
            char * const nonceStr = ((char*) buffer) + leftover.length();

            // calculate the length of the chunk in bytes
            jeaiii::to_text(nonceStr, nonce);
            const unsigned int len = strlen((char*) buffer);

            // number of bits in the entire input
            const uint64_t bits = (128 + len) * 8;

            // set the bit after the message to 1
            buffer[len] = 0b10000000;

            // append the length in bits as a 64-bit big-endian integer
            for (int i = 0; i < 8; i++) {
                buffer[63 - i] = bits >> (i * 8);
            }

            while (!done) {
                memcpy(H, H_1, 32);
                jeaiii::to_text(nonceStr, ++nonce);
            

                sha256_block_data_order(H, buffer,1);

                // output the values of the SHA context to a byte array
                for (int i = 0; i < 8; i++) {
                    digest[i*4 + 0] = H[i] >> 24;
                    digest[i*4 + 1] = H[i] >> 16;
                    digest[i*4 + 2] = H[i] >>  8;
                    digest[i*4 + 3] = H[i];
                }

                // check if the even part matches the target
                if (memcmp(digest, &binary_target[0], binary_target.size()) == 0) {
                    // check if the odd part matches the target
                    if (!is_odd || (digest[binary_target.size()] ^ odd) < 16) {
                        SHA256 sha256;
                        // printf("bits: %d", bits);
                        // printf("length of buffer: %d", len);
                        // printf("digest: %.64s\n", hash_to_string(digest).c_str());
                        // printf("what was actually hashed: %.128s%.*s\n", constant.c_str(), len, buffer);
                        // printf("string_hash(constant + nonce): %.64s\n", sha256(constant + to_string(nonce)).c_str());
                        // for (int i = 0; i < 8; i++) {
                        //     printf("H[%d]: %u\n", i, H[i]);
                        // }
                
                        done = true;
                        result = nonce;
                    }
                }
            }
        }));
    }

    // wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    return result;
}


    

