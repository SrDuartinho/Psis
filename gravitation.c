#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gravitation.h"
#include "universe-data.h"


void correct_position(float *pos) {
    if (*pos < 0) {
        *pos = WINDOW_SIZE + *pos;
    } else if (*pos >= WINDOW_SIZE) {
        *pos = *pos - WINDOW_SIZE;
    }
}

//MAX_TRASH_WORLD instead of N_TRASH to account for dynamically added trash
void new_trash_acceleration(Planet_t planets[], int total_planets,
                            Trash_t trash[], int total_trash) {
    (void)total_planets;
    (void)total_trash;
    Vector total_vector_force;
    Vector local_vector_force;
    for (int n_trash = 0; n_trash < MAX_TRASH_WORLD; n_trash++) {
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;
        for (int n_planet = 0; n_planet < PLANET_NUM; n_planet++) {
            float force_vector_x = planets[n_planet].x - trash[n_trash].position.x;
            float force_vector_y = planets[n_planet].y - trash[n_trash].position.y;
            local_vector_force = make_vector(force_vector_x, force_vector_y);
            local_vector_force.amplitude = planets[n_planet].mass /
                                           pow(local_vector_force.amplitude, 2);
            total_vector_force = add_vectors(local_vector_force,
                                             total_vector_force);
            trash[n_trash].acceleration = total_vector_force;
        }
    }
}

void new_trash_velocity(Trash_t trash[], int total_trash) {
    for (int n_trash = 0; n_trash < total_trash; n_trash++) {
        trash[n_trash].velocity.amplitude *= 0.99;
        trash[n_trash].velocity = add_vectors(trash[n_trash].velocity,
                                              trash[n_trash].acceleration);
    }
}

void new_trash_position(Trash_t trash[], int total_trash) {
    for (int n_trash = 0; n_trash < total_trash; n_trash++) {
        trash[n_trash].position.x += trash[n_trash].velocity.amplitude * 2 *
                                   cos(trash[n_trash].velocity.angle);
        trash[n_trash].position.y += trash[n_trash].velocity.amplitude *
                                   sin(trash[n_trash].velocity.angle);
        correct_position(&trash[n_trash].position.x);
        correct_position(&trash[n_trash].position.y);
    }
}


//SHIP PHYSICS
void new_ship_acceleration(Planet_t planets[], int total_planets,
                            Ship ships[], int total_ships) {
    (void)total_planets;
    (void)total_ships;
    Vector total_vector_force;
    Vector local_vector_force;
    for (int n_ship = 0; n_ship < total_ships; n_ship++) {
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;
        for (int n_planet = 0; n_planet < PLANET_NUM; n_planet++) {
            float force_vector_x = planets[n_planet].x - ships[n_ship].position.x;
            float force_vector_y = planets[n_planet].y - ships[n_ship].position.y;
            local_vector_force = make_vector(force_vector_x, force_vector_y);
             /* ---- CRITICAL SAFETY CHECK ---- */
            if (local_vector_force.amplitude < 1.0f)
                continue;   // avoid division by zero / infinity
            local_vector_force.amplitude = planets[n_planet].mass /
                                           pow(local_vector_force.amplitude, 2);
            total_vector_force = add_vectors(local_vector_force,
                                             total_vector_force);
            ships[n_ship].acceleration = total_vector_force;
        }
    }
}

void new_ship_velocity(Ship ships[], int total_ships) {
    for (int n_ship = 0; n_ship < total_ships; n_ship++) {
        ships[n_ship].velocity.amplitude *= 0.99;
        ships[n_ship].velocity = add_vectors(ships[n_ship].velocity,
                                              ships[n_ship].acceleration);
    }
}

void new_ship_position(Ship ships[], int total_ships) {
    for (int n_ship = 0; n_ship < total_ships; n_ship++) {
        ships[n_ship].position.x += ships[n_ship].velocity.amplitude * 2 *
                                   cos(ships[n_ship].velocity.angle);
        ships[n_ship].position.y += ships[n_ship].velocity.amplitude *
                                   sin(ships[n_ship].velocity.angle);
        correct_position(&ships[n_ship].position.x);
        correct_position(&ships[n_ship].position.y);
    }
}