#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "universe-data.h"



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
    // To get a "random" seed, for the planets' position
    srand(time(NULL));

    for (int i = 0; i < num_planets; i++) {
        if(i == 0){
            planets[i].x = rand() % 1000;
            planets[i].y = rand() % 1000;
            planets[i].mass = 10.0;
            planets[i].name = 'A';
            planets[i].is_garbage = 1;
            continue;
        }
        planets[i].x = rand() % 1000;
        planets[i].y = rand() % 1000;
        planets[i].mass = 10.0;
        planets[i].name = 'A' + i;
        planets[i].is_garbage= 0;
    }

     for (int i = 0; i < num_planets; i++) {
        for (int j = i + 1; j < num_planets; j++) {
            // Check if planet i and j have the same coordinates
            if (planets[i].x == planets[j].x && planets[i].y == planets[j].y) {
                // If they do, regenerate new coordinates.
                planets[j].x = rand() % 1000;
                planets[j].y = rand() % 1000;
                
                // Restart the check for the modified planet j from the beginning
                i = 0; 
                break;
            }
        }
    }
}

void trash_init(Trash_t* trash, int num_trash) {
        
    // To get a "random" seed, for the trash positions
    srand(time(NULL));
    for (int i = 0; i < num_trash; i++) {
        trash[i].position.x = rand() % 1000;
        trash[i].position.y = rand() % 1000;
        trash[i].velocity.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].mass = 1.0;
    }
}