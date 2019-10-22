#include "libdd_trojan.hpp"

#define INIT_CONTENTION 500
#define MAX_LEN 32 // Max length of message that can be sent
#define HALF_PERIOD 680000L
#define RDSEED_SPAM 8

inline int rdseed(uint64_t *seed) {
    unsigned char ok;
	asm volatile ("rdseed %0; setc %1": "=r" (*seed), "=qm" (ok));
	return (int)ok;
}

inline void hi() {
    uint64_t u64;
    unsigned long long init = __rdtsc();
    while (__rdtsc() - init < HALF_PERIOD) {
        for (int i = 0; i < RDSEED_SPAM; i++) {
            rdseed(&u64);
        }
    }
}

inline void lo() {
    unsigned long long init = __rdtsc();
    while (__rdtsc() - init < HALF_PERIOD) {
        // spin 
    }
}

// Initialize communication by creating contention
void init_channel() {
    uint64_t u64;
    for (int i = 0; i < INIT_CONTENTION; i++) {
        rdseed(&u64);
    }
}

// Callback for send
int dd_send(const uint8_t* packet, size_t size) {
    int* data = (int*) malloc(size*sizeof(int));
    bool bit;
    int idx = 0;
    if (size == CHUNK_LEN + CHUNK_ID_LEN) {
        for (int i = 0; i < 16; i++) {
            if (i > 1 && i < 8) continue;
            bit = (packet[i / 8] >> (i % 8)) & 1;
            data[idx] = bit;
            idx++;
        }
    } else {
        for (int i = 0; i < size; i++) {
            bit = (packet[i / 8] >> (i % 8)) & 1;
            data[i] = bit;
        }
    }

    for (int i = 0; i < size; i++) {
        if(data[i]) {
            hi();
            lo();
        } else {
            lo();
            hi();
        }
    }
    
    for (int i = 0; i < size; i++) {
        printf("data[%d]: %d\n", i, data[i]);
    }

    return EXIT_SUCCESS;
}


// Callback for recv
int dd_recv(uint8_t* packet, size_t *size) {
    return EXIT_SUCCESS;
}

int main() {

    init_channel();

    trojan_sess_t sess;
    init_trojan(&sess, &dd_recv, &dd_send);

    printf("Please type a message\n");
    char input[MAX_LEN];
    while (true) {
        fgets(input, MAX_LEN, stdin);
        send_msg(&sess, input);
    }

}