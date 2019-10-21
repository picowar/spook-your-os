#pragma once
#include "libdd.hpp"

typedef struct __attribute__((packed)) trojan_sess_t {
    uint8_t csid : 4;
    size_t mtu;
    size_t chunk_seq_lim; 
    recv_buffer_t dd_recv;
    send_buffer_t dd_send;
    fitf_t active_msg;
    uint8_t current_id : 2; // alternates between 10, 11
} trojan_sess_t;

int init_trojan(trojan_sess_t* sess, 
                size_t mtu, 
                recv_buffer_t recv_cb,
                send_buffer_t send_cbk);

int send_msg(trojan_sess_t* sess, char* buf);

int send_fitf(trojan_sess_t* sess, fitf_t* fitf);

int send_msg_chunks(trojan_sess_t* sess);

int init_fitf(trojan_sess_t* sess, fitf_t* fitf, uint16_t msg_len);

