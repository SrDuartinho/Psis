#include <stdio.h>
#include <stdlib.h>

#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

//Constants (act as compile-time maxima / defaults)
#define PLANET_NUM 30       //Total number of planets (max/default)
#define N_TRASH 100          //Total number of trash (default initial amount)
#define WINDOW_SIZE 2000    //Window size (pixels)
#define SHIP_CAPACITY 50     //Maximum ammount of trash allowed in the ships
#define MAX_TRASH_WORLD 100  //Maximum ammount of trash allowed in the world
extern int RECYCLE_PLANET_INDEX;  // declaration only

// Runtime configuration (loaded from .conf, falls back to defaults above)
typedef struct {
    int planet_num;
    int trash_amount;
    int window_size;
    int ship_capacity;
    int max_trash_world;
} GameConfig;

extern GameConfig g_config;
int load_game_config(const char *path);

//Type definitions
typedef struct {
    float amplitude;
    float angle;
} Vector;

typedef struct {
    float x;
    float y;
} Position_t;

typedef struct {
    Position_t position;
    Vector velocity;
    Vector acceleration;
    float mass;
} Trash_t;

typedef struct {
    Trash_t trash[N_TRASH];
    Position_t position;
    Vector velocity;
    Vector acceleration;
    float mass;
    int trash_count;
    char ch;
} Ship;

typedef struct {
    Trash_t trash[N_TRASH];
    int trash_count;
    int x;
    int y;
    float mass;
    char name;
    int is_garbage;
    Ship ship;
    int ship_assigned;
} Planet_t;

Vector make_vector(float x, float y);
Vector add_vectors(Vector v1, Vector v2);
void planets_init(Planet_t* planets, int num_planets);
void trash_init(Trash_t* trash, int num_trash);
void init_recycle_index(void);
int generate_new_trash(Trash_t* trash, int current_trash, int x, int y);
int trash_planet_collision(Trash_t* trash, Planet_t* planet);

#endif