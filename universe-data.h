#include <stdio.h>
#include <stdlib.h>

#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

//Constants 
#define PLANET_NUM 10       //Total number of planets
#define N_TRASH 10          //Total number of trash 
#define WINDOW_SIZE 1000    //Window size
#define SHIP_CAPACITY 5     //Maximum ammount of trash allowed in the ships
#define MAX_TRASH_WORLD 20  //Maximum ammount of trash allowed in the world
extern int RECYCLE_PLANET_INDEX;  // declaration only

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