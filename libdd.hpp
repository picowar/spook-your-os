#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <x86intrin.h>
#include <unistd.h>
#include <time.h>

#include "libdd_trojan.hpp"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RDSEED_SUCCESS 1
#define CHUNK_LEN 8      // number of bits transferred in one chunk excl id
#define CHUNK_ID_LEN 2   // number of bits in chunk id
#define FITF_LEN 8       // number of bits in fitf

typedef int (*recv_buffer_t)(uint8_t*, size_t*);

typedef int (*send_buffer_t)(const uint8_t*, size_t);

typedef struct __attribute__((packed)) fitf_t {
    uint8_t msg_len;
} fitf_t;

typedef struct __attribute__((packed)) chunk_t {
    uint8_t id : 2;
    uint8_t data[CHUNK_LEN];
} chunk_t;
