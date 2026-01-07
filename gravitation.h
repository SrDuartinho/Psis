#include <stdio.h>
#include <stdlib.h>
#include "universe-data.h"

void correct_position(float *pos);

void new_trash_acceleration(Planet_t planets[], int total_planets, Trash_t trash[], int total_trash);

void new_trash_velocity(Trash_t trash[], int total_trash);

void new_trash_position(Trash_t trash[], int total_trash);

void new_ship_acceleration(Planet_t planets[], int total_planets, Ship ships[], int total_ships);

void new_ship_velocity(Ship ships[], int total_ships);

void new_ship_position(Ship ships[], int total_ships);