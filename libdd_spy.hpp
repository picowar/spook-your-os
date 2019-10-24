#pragma once
#include "libdd.hpp"

#define DEFAULT_MSG_LEN 8
#define FITF_FAIL -1
#define CHUNK_FAIL -1

typedef enum state_t {
    Inactive, 
    InitialTransfer,
    Reconstruction
} state_t;

typedef struct __attribute__((packed)) spy_sess_t {
    fitf_t active_file;
    send_buffer_t dd_send;
    recv_buffer_t dd_recv;
    state_t spy_state;
    bool finished;

} spy_sess_t;

int init_spy(spy_sess_t* sess,  
            recv_buffer_t recv_cbk,
            send_buffer_t send_cbk);

int start_spy_sess(spy_sess_t* sess);

int process_fitf(spy_sess_t *sess, uint8_t *fitf);

int process_chunk(spy_sess_t *sess, uint8_t *packet, bool flag);