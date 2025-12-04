#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <zmq.h>
#include "messages.pb.h"

extern "C" {
#include "communication.h"
}

// Keep C linkage for the functions declared in communication.h
extern "C" void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:5555");
    (void)response;
    return responder;
}

extern "C" void * create_client_channel(char * server_ip_addr){
    char server_zmq_addr[100];
    snprintf(server_zmq_addr, sizeof(server_zmq_addr), "tcp://%s:5555", server_ip_addr);
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, server_zmq_addr);
    return requester;
}

static bool recv_bytes(void* fd, std::string &out) {
    // receive into a fixed buffer
    char buf[8192];
    int n = zmq_recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) return false;
    out.assign(buf, buf + n);
    return true;
}

static bool send_bytes(void* fd, const std::string &data) {
    int rc = zmq_send(fd, data.data(), data.size(), 0);
    return rc == (int)data.size();
}

extern "C" void read_message (void * fd, char * message_type, char * c, direction_t *d ){
    std::string data;
    if (!recv_bytes(fd, data)) return;
    universe::Envelope env;
    if (!env.ParseFromString(data)) return;
    if (env.has_connect()){
        strcpy(message_type, "CONNECT");
        std::string ch = env.connect().ch();
        c[0] = ch.size() ? ch[0] : '\0';
        d[0] = '\0';
    } else if (env.has_move()){
        strcpy(message_type, "MOVE");
        std::string chs = env.move().ch();
        std::string dir = env.move().dir();
        c[0] = chs.size() ? chs[0] : '\0';
        d[0] = dir.size() ? dir[0] : '\0';
    } else if (env.has_response()){
        strcpy(message_type, "RESPONSE");
        std::string txt = env.response().text();
        // place text into message_type? keep as RESPONSE and client will call receive_response
        c[0] = '\0';
        d[0] = '\0';
    } else {
        message_type[0] = '\0';
        c[0] = '\0';
        d[0] = '\0';
    }
}

extern "C" void send_response (void * fd, char * message){
    universe::Envelope env;
    universe::Response *r = env.mutable_response();
    r->set_text(message);
    std::string out;
    env.SerializeToString(&out);
    send_bytes(fd, out);
}

extern "C" void send_connection_message(void * fd, char ch){
    universe::Envelope env;
    universe::Connect *cmsg = env.mutable_connect();
    std::string s(1, ch);
    cmsg->set_ch(s);
    std::string out;
    env.SerializeToString(&out);
    send_bytes(fd, out);
}

extern "C" void send_movement_message(void * fd, char ch, direction_t d){
    universe::Envelope env;
    universe::Move *m = env.mutable_move();
    std::string s(1, ch);
    std::string dir(1, d);
    m->set_ch(s);
    m->set_dir(dir);
    std::string out;
    env.SerializeToString(&out);
    send_bytes(fd, out);
}

extern "C" void receive_response (void * fd, char * message){
    std::string data;
    if (!recv_bytes(fd, data)) { message[0] = '\0'; return; }
    universe::Envelope env;
    if (!env.ParseFromString(data)) { message[0] = '\0'; return; }
    if (env.has_response()){
        std::string txt = env.response().text();
        strncpy(message, txt.c_str(), 99);
        message[99] = '\0';
    } else {
        message[0] = '\0';
    }
}
