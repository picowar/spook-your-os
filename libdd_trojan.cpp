#include "libdd_trojan.hpp"

int init_trojan(trojan_sess_t *sess,
        size_t mtu,
        recv_buffer_t recv_cbk,
        send_buffer_t send_cbk
        ) {

    memset(sess, 0, sizeof(sess));
    sess->dd_recv = recv_cbk;
    sess->dd_send = send_cbk;
    sess->mtu = mtu;
    sess->current_id = 2; // either 2 or 3 alternates
    sess->chunk_seq_lim = CHUNK_SEQ_LIMIT;
    return EXIT_SUCCESS;

}