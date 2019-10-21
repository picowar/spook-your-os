#include "libdd.hpp"

#define INIT_CONTENTION 500
#define MAX_LEN 32

inline int rdseed(uint64_t *seed) {
    unsigned char ok;
	asm volatile ("rdseed %0; setc %1": "=r" (*seed), "=qm" (ok));
	return (int)ok;
}

// Initialize communication by creating contention
void init_channel() {
    uint64_t u64;
    for (int i = 0; i < INIT_CONTENTION; i++) {
        rdseed(&u64);
    }
}

// Callback for send
int dd_send(uint8_t* packet, size_t size) {

}


// Callback for recv
int dd_recv(uint8_t* packet, size_t *size) {

}

int main() {

    init_channel();

    trojan_sess_t sess;
    init_trojan(&sess, MAX_MTU, &dd_recv, &dd_send);

    printf("Please type a message\n");
    char input[MAX_LEN];
    while (true) {
        fgets(input, MAX_LEN, stdin);
        dd_send_msg(&sess, input);
    }

}