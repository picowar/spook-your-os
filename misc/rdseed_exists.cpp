/* These are bits that are OR’d together */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <x86intrin.h>
#include <unistd.h>
#include <time.h>

#define DRNG_NO_SUPPORT	0x0	/* For clarity */
#define DRNG_HAS_RDRAND	0x1
#define DRNG_HAS_RDSEED	0x2
#define DRNG_SUCCESS 0
#define DRNG_NOT_READY -1
/* Maximum retry value of rdseed instruction in a single call*/
#define MAX_RETRY_LIMIT 2

#define RDSEED_SPAM 500
#define RDTSC_WAIT 4000
#define BILLION 1E9

// optional wrapper if you don't want to just use __rdtsc() everywhere
inline
unsigned long long readTSC() {
    // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
    return __rdtsc();
    // _mm_lfence();  // optionally block later instructions until rdtsc retires
}

typedef struct cpuid_struct {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
} cpuid_t;


void cpuid (cpuid_t *info, unsigned int leaf, unsigned int subleaf)
{
	asm volatile("cpuid"
	: "=a" (info->eax), "=b" (info->ebx), "=c" (info->ecx), "=d" (info->edx)
	: "a" (leaf), "c" (subleaf)
	);
}


int _is_intel_cpu ()
{
	static int intel_cpu= -1;
	cpuid_t info;

	if ( intel_cpu == -1 ) {
		cpuid(&info, 0, 0);

		if (
			memcmp((char *) &info.ebx, "Genu", 4) ||
			memcmp((char *) &info.edx, "ineI", 4) ||
			memcmp((char *) &info.ecx, "ntel", 4)
		) {
			intel_cpu= 0;
		} else {
			intel_cpu= 1;
		}
	}

	return intel_cpu;
}


int get_drng_support ()
{
	static int drng_features= -1;

	/* So we don't call cpuid multiple times for 
	 * the same information */

	if ( drng_features == -1 ) {
		drng_features= DRNG_NO_SUPPORT;

		if ( _is_intel_cpu() ) {
			cpuid_t info;

			cpuid(&info, 1, 0);

			if ( (info.ecx & 0x40000000) == 0x40000000 ) {
				drng_features|= DRNG_HAS_RDRAND;
			}

			cpuid(&info, 7, 0);

			if ( (info.ebx & 0x40000) == 0x40000 ) {
				drng_features|= DRNG_HAS_RDSEED;
			}
		} 
	}

	return drng_features;
}

int rdseed64_step(uint64_t *seed)
{
	unsigned char ok;

	asm volatile ("rdseed %0; setc %1": "=r" (*seed), "=qm" (ok));

	return (int)ok;
} 

int rdseed16_step (uint16_t *seed)
{
	unsigned char ok;

	asm volatile ("rdseed %0; setc %1"
		: "=r" (*seed), "=qm" (ok));

	return (int) ok;
}

int rdseed_16(uint16_t* x, int retry)
{
	int retry_counter = retry;
	if (rdseed16_step(x))
		return DRNG_SUCCESS;
	else
	{
		retry_counter = retry;
		while (retry_counter > 0)
		{
			retry_counter--;
			if (rdseed16_step(x))
				return DRNG_SUCCESS;
		}

		return DRNG_NOT_READY;
	}
}

inline void rdtsc_wait(unsigned long long n) {
    unsigned long long init = __rdtsc();
    while (__rdtsc() - init < n) {
        // spin 
    }
}


int rdseed_64(uint64_t* x)
{
	if (rdseed64_step(x))
		return DRNG_SUCCESS;
	else 
		return DRNG_NOT_READY;
}

inline void rdseed_spam(int n) {
    uint64_t u64;
    for (int i = 0; i < n; i++) {
        rdseed64_step(&u64);
    }
}

inline void rdtsc_wait(int n) {
    for (int i = 0; i < n; i++) {
        __rdtsc();
    }
}


int main() {
	uint64_t u64;
	unsigned long long t;
	int r;

	/*
	Write a benchmark tool that executes rdseed continuously, 
	*/
	int fails = 0;
	int N = 1000;

	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = 0;
	int rip = 0;
	unsigned long avg_time = 0;

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);
	rdseed_spam(RDSEED_SPAM); // hi
    // rdtsc_wait(RDTSC_WAIT); // lo
	clock_gettime(CLOCK_REALTIME, &end);
	long diff = (end.tv_nsec - start.tv_nsec) + (end.tv_sec - start.tv_sec) * BILLION;
	printf("time for rdseed_spam: %ld\n", diff);

	clock_gettime(CLOCK_REALTIME, &start);
	rdtsc_wait(RDTSC_WAIT);
	// rdseed_spam(RDSEED_SPAM);
	clock_gettime(CLOCK_REALTIME, &end);
	diff = (end.tv_nsec - start.tv_nsec) + (end.tv_sec - start.tv_sec) * BILLION;
	printf("time for rdseed_wait: %ld\n", diff);



	

	t = __rdtsc();
	for (int j = 0; j < 100; j++) {
		__rdtsc();
	}
	t = __rdtsc() - t;
	printf("rdtsc for T_Prime: %llu\n", t);

	t = __rdtsc();
	for (int k = 0 ; k < 1000; k++) {
		r = rdseed64_step(&u64);
	}
	t = __rdtsc() - t;
	printf("rdtsc for 1000 rdseed: %llu\n", t);

	t = __rdtsc();
	for (int k = 0 ; k < 10000; k++) {
		__rdtsc();
	}
	t = __rdtsc() - t;
	printf("rdtsc for 10k rdtsc: %llu\n", t);



	int count_high = 0;
	for (int k = 0; k < 20; k++) {
		if (rdseed64_step(&u64) != 1) {
			count_high++;
		}
		__rdtsc();
		__rdtsc();
		__rdtsc();
		__rdtsc();
		__rdtsc();
    }

	t = __rdtsc();
	r = rdseed64_step(&u64);
	t = __rdtsc() - t;
	printf("rdtsc for a T_rdseed: %llu\n", t);

	t = __rdtsc();
	__rdtsc();
	t = __rdtsc() - t;
	printf("rdtsc for two rdtsc: %llu\n", t);


	t = __rdtsc();
	asm volatile("pause");
	t = __rdtsc() - t;
	printf("rdtsc for a NOP: %llu\n", t);
	int miss[10];

	for (int i = 0; i < N; i++) {

		// Execute rdseed once (no retries), print the output
		// t = readTSC();
		r = rdseed64_step(&u64);
		// t = readTSC() - t;
		// avg_time += t;

		// printf("rdtsc output for itr: %d: %llu\n", i, t);
		
		if (r != 1) {
			//printf("rdseed instruction failed with code %d for i = %d\n", r, i);
			fails++;
			//printf("rdtsc o(const struct timespec[]){{0, 1}}utput for fail: %llu\n", t);
		} 

		// rip += 4;
		// t = __rdtsc();
		// for (int j = 0; j < 1; j++) {
		// 	__rdtsc();
		// }
		// t = __rdtsc() - t;
		
		// asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
		// asm volatile("nop");
		// asm volatile("nop");
		// asm volatile("pause; pause; pause;");
		t = readTSC();
		miss[0] = 1;
		t = readTSC() - t;
		printf("heya: %llu\n", t);
		// nanosleep(&tim, NULL);
	}

	// printf("avg_time: %lu\n", avg_time/N);
	// printf("readtsc for nop: %llu\n", t);


	printf("fails: %d, total: %d\n", fails, N);
}

