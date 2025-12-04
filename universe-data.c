#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "universe-data.h"

int RECYCLE_PLANET_INDEX;  // definition

void init_recycle_index(void) {
    RECYCLE_PLANET_INDEX = rand() % PLANET_NUM;
}

Vector make_vector(float x, float y) {
    Vector v;
    v.amplitude = sqrt(x * x + y * y);
    v.angle = atan2(y, x);
    return v;
}

Vector add_vectors(Vector v1, Vector v2) {
    // Convert polar to cartesian
    float x1 = v1.amplitude * cos(v1.angle);
    float y1 = v1.amplitude * sin(v1.angle);
    float x2 = v2.amplitude * cos(v2.angle);
    float y2 = v2.amplitude * sin(v2.angle);

    // Add cartesian components
    float sum_x = x1 + x2;
    float sum_y = y1 + y2;

    // Convert back to polar and return
    return make_vector(sum_x, sum_y);
}


void planets_init(Planet_t* planets, int num_planets) {
    for (int i = 0; i < num_planets; i++) {
        if(i == RECYCLE_PLANET_INDEX){
            planets[i].x = rand() % WINDOW_SIZE;
            planets[i].y = rand() % WINDOW_SIZE;
            planets[i].trash_count = 0;
            planets[i].mass = 10.0;
            planets[i].name = 'A'+ i;
            planets[i].is_garbage = 1;
            // initialize planet's default ship
            planets[i].ship.ch = planets[i].name;
            planets[i].ship.position.x = planets[i].x;
            planets[i].ship.position.y = planets[i].y;
            planets[i].ship.trash_count = 0;
            planets[i].ship_assigned = 0;
            continue;
        }
        planets[i].x = rand() % WINDOW_SIZE;
        planets[i].y = rand() % WINDOW_SIZE;
        planets[i].trash_count = 0;
        planets[i].mass = 10.0;
        planets[i].name = 'A' + i;
        planets[i].is_garbage= 0;
        planets[i].ship.ch = planets[i].name;
        planets[i].ship.position.x = planets[i].x;
        planets[i].ship.position.y = planets[i].y;
        planets[i].ship.trash_count = 0;
        planets[i].ship_assigned = 0;
    }

     for (int i = 0; i < num_planets; i++) {
        for (int j = i + 1; j < num_planets; j++) {
            // Check if planet i and j have the same coordinates
            if (planets[i].x == planets[j].x && planets[i].y == planets[j].y) {
                // If they do, regenerate new coordinates.
                planets[j].x = rand() % WINDOW_SIZE;
                planets[j].y = rand() % WINDOW_SIZE;
                
                // Restart the check for the modified planet j from the beginning
                i = 0; 
                break;
            }
        }
    }
}

void trash_init(Trash_t* trash, int num_trash) {
    for (int i = 0; i < num_trash; i++) {
        trash[i].position.x = rand() % WINDOW_SIZE;
        trash[i].position.y = rand() % WINDOW_SIZE;
        trash[i].velocity.amplitude = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.angle = 0;
        trash[i].mass = 1.0;
    }
}

int generate_new_trash(Trash_t* trash, int current_trash, int x, int y){
    if (current_trash >= MAX_TRASH_WORLD) {
        printf("Trash overflow.\n");
        return -1;
    }
    else{
        trash[current_trash].position.x = x + 2;
        trash[current_trash].position.y = y + 2;
        trash[current_trash].velocity.amplitude = 0;
        trash[current_trash].velocity.angle = 0;
        trash[current_trash].acceleration.amplitude = 0;
        trash[current_trash].acceleration.angle = 0;
        trash[current_trash].mass = 1.0;
        return 1;
    }
}

int trash_planet_collision(Trash_t* trash, Planet_t* planet){
    float dx = trash->position.x - planet->x;
    float dy = trash->position.y - planet->y;
    
    /*
    float distance = sqrt(dx * dx + dy * dy);
    //float collision_distance = 20.0 + 4.0; // planet radius + trash radius
    float collision_distance = 1.0; // When their centers are 1 unit apart (touching each other)

    if (distance <= collision_distance) {
        return 1; // Collision detected
    }
    */

    //fabs = abs, for float values
    if (fabs(dx) < 1 && fabs(dy) < 1) {
        return 1; // Collision detected
    }
    return 0; // No collision
}
    