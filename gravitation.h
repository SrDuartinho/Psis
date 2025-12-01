#include <stdio.h>
#include <stdlib.h>
#include "universe-data.h"

void correct_position(float *pos);

void new_trash_acceleration(Planet_t planets[], int total_planets, Trash_t trash[], int total_trash);

void new_trash_velocity(Trash_t trash[], int total_trash);

void new_trash_position(Trash_t trash[], int total_trash);
