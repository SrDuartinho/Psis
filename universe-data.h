#include <stdio.h>
#include <stdlib.h>

#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

//Constants
#define PLANET_NUM 2
#define N_TRASH 20

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
    int trash_count;
    int x;
    int y;
    float mass;
    char name;
} Planet_t;


typedef struct {
    Trash_t trash[N_TRASH];
    Position_t position;
    int trash_count;
    char ch;
} Ship;


Vector make_vector(float x, float y);
Vector add_vectors(Vector v1, Vector v2);

#endif
