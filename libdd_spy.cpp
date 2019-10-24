#include "libdd_spy.hpp"

int init_spy(spy_sess_t *sess, 
            recv_buffer_t recv_cbk,
            send_buffer_t send_cbk) {

    memset(sess, 0, sizeof(sess));
    sess->dd_send = send_cbk;
    sess->dd_recv = recv_cbk;
    sess->finished = false;
    sess->spy_state = Inactive;

    return EXIT_SUCCESS;
}

int start_spy_sess(spy_sess_t *sess) {
    uint8_t fitf;
    uint8_t data[MAX_MTU];
    size_t bytes_recv;
    int len;

    // Receiving header from trojan
    sess->dd_recv(&fitf, (size_t*) bytes_recv, true);
    printf("received header\n");
    len = process_fitf(sess, &fitf);

    if (len < 0) {
        printf("Length cannot be determined, defaulting to 8-bytes\n");
        len = DEFAULT_MSG_LEN;
    }

    printf("len: %d\n", len);

    // receiving chunks from trojan
    for (int i = 0; i < len; i++) {
        memset(data, 0, MAX_MTU);
        sess->dd_recv(data, (size_t*) bytes_recv, false);
        process_chunk(sess, data, i%2);
    }

    return EXIT_SUCCESS;
}

int process_fitf(spy_sess_t *sess, uint8_t *fitf) {
    if (sess == NULL || fitf == NULL) return FITF_FAIL;
    
    // if initial bit is not 1 then fail
    if (fitf[0] != 1) return FITF_FAIL;
    int sum = 0; 
    for (int i = 0; i < 7; i++) {
        sum += ((fitf[i+1]) * pow(2, i));
    }
    return sum;
}

int process_chunk(spy_sess_t *sess, uint8_t *packet, bool flag) {
    // if first bit is not 1 fail
    if (packet[0] != 1) return CHUNK_FAIL;

    // if second bit is not the correct alternation of 1 or 0 fail
    if (packet[1] != flag) return CHUNK_FAIL;

    // then print the other stuff
    char* temp = new char[9];
    for (int j = 0; j < 8; j++) {
        temp[j] = packet[(9-j)-1];
    }
    temp[8] = 0;
    char c = strtol(temp, 0, 2);
    printf("%c", c);
    return EXIT_SUCCESS;
}   


    

