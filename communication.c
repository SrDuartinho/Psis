#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "communication.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  


#include <zmq.h>


void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:5555");
    (void)response;
    return responder;
}


void read_message (void * fd, char * message_type, char * c, direction_t *d ){
    char message[100];
    int n = zmq_recv (fd, message, 99, 0);
    if (n <= 0) {
        message_type[0] = '\0';
        c[0] = '\0';
        d[0] = '\0';
        return;
    }
    if (n < 99) message[n] = '\0'; else message[99] = '\0';
    d[0]='\0';
    sscanf(message, "%s %c %c", message_type, c, d);
    return;
}


void send_response (void * fd, char * message){
    zmq_send(fd, message, strlen(message), 0);
}


void * create_client_channel(char * server_ip_addr){
    char server_zmq_addr[100];
    sprintf(server_zmq_addr, "tcp://%s:5555", server_ip_addr);
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    return requester;
}


void send_connection_message(void * fd, char ch){
    char message[100];
    sprintf(message, "CONNECT %c", ch);
    zmq_send (fd, message, strlen(message), 0);
}

void send_movement_message(void * fd, char ch, direction_t d){
    char message[100];
    sprintf(message, "MOVE %c %c", ch, d);
    zmq_send (fd, message, strlen(message), 0);
}
void receive_response (void * fd, char * message){
    int n = zmq_recv (fd, message, 99, 0);
    if (n <= 0) { message[0] = '\0'; return; }
    if (n < 99) message[n] = '\0'; else message[99] = '\0';
}
