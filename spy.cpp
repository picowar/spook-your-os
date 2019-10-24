#include "libdd_spy.hpp"

#define CONTENTION_DETECT_COUNT 10
#define CONTENTION_WAIT 25
#define STATE_CHANGE_COUNT 3
#define PERIOD 1300000L
#define PROBE_WAIT 15

uint64_t u64;

inline int rdseed(uint64_t *seed) {
    unsigned char ok;
	asm volatile ("rdseed %0; setc %1": "=r" (*seed), "=qm" (ok));
	return (int)ok;
}

inline void rdtsc_wait(int n) {
    for (int i = 0; i < n; i++) {
        __rdtsc();
    }
}

int dd_send(const uint8_t* packet, size_t size) {
    return EXIT_SUCCESS;
}

int dd_recv(uint8_t *packet, size_t *size, bool fitf) {

    int curr_state = 1;
    int red_flag = 0;
    int bits_recv = 0;
    unsigned long long curr, before = 0;
    int bits_to_recv;

    if (fitf) {
        bits_to_recv = FITF_LEN;
    } else {
        bits_to_recv = CHUNK_ID_LEN + CHUNK_LEN;
    }

    while (bits_recv < bits_to_recv) {
        if (rdseed(&u64) == RDSEED_SUCCESS) {
            // if zero is received
            if (curr_state == 1 && red_flag < STATE_CHANGE_COUNT) {
                // could be noise
                red_flag++;
            } else if (curr_state == 1 && red_flag >= STATE_CHANGE_COUNT) {
                // definitely not noise, change state
                curr_state = 0;
                red_flag = 0; 
                if (curr - before > PERIOD || bits_recv == 0) {
                    before = curr;
                    packet[bits_recv] = '1';
                    bits_recv++;
                }
            } else if (curr_state == 0 && red_flag <= STATE_CHANGE_COUNT) {
                // status-quo
                red_flag = 0;
            } else {
                printf("Weird shit happened, exiting!");
                printf("curr_state: %d, red_flag: %d, zero\n", curr_state, red_flag);
                return EXIT_FAILURE;
            }
        } else {
            // if one is received

            if (curr_state == 0 && red_flag < STATE_CHANGE_COUNT) {
                // could be noise
                red_flag++;
            } else if (curr_state == 0 && red_flag >= STATE_CHANGE_COUNT) {
                // definitely not noise, change state
                curr_state = 1;
                red_flag = 0; 
                curr = __rdtsc();
                if (curr - before > PERIOD || bits_recv == 0) {
                    before = curr;
                    packet[bits_recv] = '0';
                    bits_recv++;
                }
            } else if (curr_state == 1 && red_flag <= STATE_CHANGE_COUNT) {
                // status-quo
                red_flag = 0;
            } else {
                printf("Weird shit happened, exiting!");
                printf("curr_state: %d, red_flag: %d, one\n", curr_state, red_flag);
                return EXIT_FAILURE;
            }
        }
        rdtsc_wait(PROBE_WAIT);        
    }

    *size = bits_recv;
    return EXIT_SUCCESS;
}

inline void init_channel() {
    int miss_counter = 0;
    uint64_t u64;
    while (miss_counter < CONTENTION_DETECT_COUNT) {
        if (rdseed(&u64) != RDSEED_SUCCESS) {
            // rdseed failed
            miss_counter++;
        } else {
            miss_counter = 0;
        }

        rdtsc_wait(CONTENTION_WAIT);
    }
    printf("Spy is now listening!\n");
}

int main() {

    spy_sess_t sess;
    init_spy(&sess, &dd_recv, &dd_send);

    while (true) {
        init_channel();
        start_spy_sess(&sess);
    }

    return EXIT_SUCCESS;
}