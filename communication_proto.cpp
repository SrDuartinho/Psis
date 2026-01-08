#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <zmq.h>
#include "proto/messages.pb.h"

extern "C" {
#include "communication.h"
}

// Keep C linkage for the functions declared in communication.h
extern "C" void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:5555");
    (void)response;
    // Set receive timeout to 10ms to allow responsive rendering and quit handling
    int timeout = 10;
    zmq_setsockopt(responder, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
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
    char buf[65536]; // Increased buffer size to handle larger messages of game state
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

/**
 * Sends the current game state to the fd pointer (in this case the client socket).
 * @param fd The file descriptor (socket) to send the data to.
 * @param ships Array of Ship structures representing the ships in the game.
 * @param n_ships Number of ships in the ships array.
 * @param planets Array of Planet_t structures representing the planets in the game.
 * @param n_planets Number of planets in the planets array.
 * @param trash Array of Trash_t structures representing the trash in the game.
 * @param n_trash Number of trash items in the trash array.
 */
extern "C" void send_game_state(void * fd, Ship ships[], int n_ships, 
                                  Planet_t planets[], int n_planets,
                                  Trash_t trash[], int n_trash) {
    universe::Envelope env;
    universe::GameState *gs = env.mutable_game_state();
    
    // Add ships
    for (int i = 0; i < n_ships; i++) {
        universe::Ship *s = gs->add_ships();
        s->set_ch(std::string(1, ships[i].ch));
        s->mutable_pos()->set_x(ships[i].position.x);
        s->mutable_pos()->set_y(ships[i].position.y);
        s->set_trash_count(ships[i].trash_count);
    }
    
    // Add planets
    for (int i = 0; i < n_planets; i++) {
        universe::Planet *p = gs->add_planets();
        p->set_name(std::string(1, planets[i].name));
        p->mutable_pos()->set_x(planets[i].x);
        p->mutable_pos()->set_y(planets[i].y);
        p->set_trash_count(planets[i].trash_count);
        p->set_is_garbage(planets[i].is_garbage);
    }
    
    // Add trash
    for (int i = 0; i < n_trash; i++) {
        universe::Trash *t = gs->add_trash();
        t->mutable_pos()->set_x(trash[i].position.x);
        t->mutable_pos()->set_y(trash[i].position.y);
    }
    
    gs->set_total_trash(n_trash);
    
    std::string out;
    env.SerializeToString(&out);
    send_bytes(fd, out);
}

/**
 * Receives the current game state from the fd pointer (in this case the server socket).
 * @param fd The file descriptor (socket) to receive the data from.
 * @param ships Array of Ship structures to populate with the received ships.
 * @param n_ships Pointer to an integer to store the number of ships received.
 * @param planets Array of Planet_t structures to populate with the received planets.
 * @param n_planets Pointer to an integer to store the number of planets received.
 * @param trash Array of Trash_t structures to populate with the received trash items.
 * @param n_trash Pointer to an integer to store the number of trash items received.
 * @return 1 on success, 0 on failure.
 */
extern "C" int receive_game_state(void * fd, Ship ships[], int *n_ships,
                                    Planet_t planets[], int *n_planets,
                                    Trash_t trash[], int *n_trash) {
    std::string data;
    //Read the raw data from the socket
    if (!recv_bytes(fd, data)) return 0;
    
    universe::Envelope env;
    // Parse the received data to the original structures
    if (!env.ParseFromString(data)) return 0;
    
    // Ensure that the envelope contains a game state
    if (!env.has_game_state()) return 0;
    
    const universe::GameState &gs = env.game_state();
    
    // Parse ships
    *n_ships = gs.ships_size();
    for (int i = 0; i < *n_ships && i < 100; i++) {
        ships[i].ch = gs.ships(i).ch()[0];
        ships[i].position.x = gs.ships(i).pos().x();
        ships[i].position.y = gs.ships(i).pos().y();
        ships[i].trash_count = gs.ships(i).trash_count();
    }
    
    // Parse planets
    *n_planets = gs.planets_size();
    for (int i = 0; i < *n_planets; i++) {
        planets[i].name = gs.planets(i).name()[0];
        planets[i].x = gs.planets(i).pos().x();
        planets[i].y = gs.planets(i).pos().y();
        planets[i].trash_count = gs.planets(i).trash_count();
        planets[i].is_garbage = gs.planets(i).is_garbage();
    }
    
    // Parse trash
    *n_trash = gs.trash_size();
    for (int i = 0; i < *n_trash && i < MAX_TRASH_WORLD; i++) {
        trash[i].position.x = gs.trash(i).pos().x();
        trash[i].position.y = gs.trash(i).pos().y();
    }
    
    return 1;
}