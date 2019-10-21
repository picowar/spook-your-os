#include "libdd.hpp"

#define CONTENTION_DETECT_COUNT 10
#define CONTENTION_WAIT 25

inline void init_spy() {
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

    init_spy();
}