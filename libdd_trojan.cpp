#include "libdd_trojan.hpp"

int init_trojan(trojan_sess_t *sess,
        recv_buffer_t recv_cbk,
        send_buffer_t send_cbk
        ) {

    memset(sess, 0, sizeof(sess));
    sess->dd_recv = recv_cbk;
    sess->dd_send = send_cbk;
    sess->current_id = 2; // either 2 or 3 alternates
    return EXIT_SUCCESS;
}

int send_msg(trojan_sess_t *sess, const char *msg) {
    if (sess == NULL || msg == NULL) return EXIT_FAILURE;

    uint8_t msg_len = ((uint8_t) strlen(msg))-1;

    // initialize header
    fitf_t fitf;
    memset(&fitf, 0, sizeof(fitf_t));
    init_fitf(sess, &fitf, msg_len);
    sess->active_msg = fitf;

    // send file transfer header
    // sent at the start of a session
    send_fitf(sess, &fitf);
    
    // send the msg
    send_msg_chunks(sess, msg);

    return EXIT_SUCCESS;
}

int init_fitf(trojan_sess_t *sess, fitf_t* fitf, uint8_t msg_len) {
    fitf->msg_len = msg_len;
    return EXIT_SUCCESS;
}

int send_fitf(trojan_sess_t *sess, fitf_t* fitf) {
    sess->dd_send((uint8_t*) fitf, FITF_LEN);
    return EXIT_SUCCESS;
}

int send_msg_chunks(trojan_sess_t *sess, const char *msg) {

    uint8_t msg_len = sess->active_msg.msg_len;
    uint8_t bytes_read = 0;

    while (bytes_read < msg_len) {
        chunk_t curr_chunk;
        memset(&curr_chunk, 0, sizeof(chunk_t));
        curr_chunk.id = sess->current_id;
        memcpy(curr_chunk.data, msg + bytes_read, CHUNK_LEN/8);
        sess->dd_send((uint8_t*) &curr_chunk, CHUNK_LEN + CHUNK_ID_LEN);
        bytes_read++;
        if (sess->current_id == 2) {
            ++(sess->current_id);
        } else {
            --(sess->current_id);
        }
    }

    return EXIT_SUCCESS;
}

