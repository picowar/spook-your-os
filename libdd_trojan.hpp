#pragma once
#include "libdd.hpp"

typedef struct __attribute__((packed)) trojan_sess_t {
    recv_buffer_t dd_recv;
    send_buffer_t dd_send;
    fitf_t active_msg;
    uint8_t current_id : 2; // alternates between 10, 11
} trojan_sess_t;

int init_trojan(trojan_sess_t* sess,  
                recv_buffer_t recv_cbk,
                send_buffer_t send_cbk);

int send_msg(trojan_sess_t* sess, const char* buf);

int send_fitf(trojan_sess_t* sess, fitf_t* fitf);

int send_msg_chunks(trojan_sess_t* sess);

int init_fitf(trojan_sess_t* sess, fitf_t* fitf, uint8_t msg_len);

int send_fitf(trojan_sess_t *sess, fitf_t* fitf);

int send_msg_chunks(trojan_sess_t *sess, const char *msg);

