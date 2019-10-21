#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <x86intrin.h>
#include <unistd.h>
#include <time.h>
#include <iostream>

#include "libdd_utils.hpp"
#include "libdd_trojan.hpp"
#include "libdd_spy.hpp"
#include "libdd_packets.hpp"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RDSEED_SUCCESS 1

