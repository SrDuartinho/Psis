#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <libconfig.h>
#include "universe-data.h"

int RECYCLE_PLANET_INDEX;  // definition
GameConfig g_config = {
    .planet_num = PLANET_NUM,
    .trash_amount = N_TRASH,
    .window_size = WINDOW_SIZE,
    .ship_capacity = SHIP_CAPACITY,
    .max_trash_world = MAX_TRASH_WORLD
};

static int clamp_int(int value, int min_v, int max_v) {
    if (value < min_v) return min_v;
    if (value > max_v) return max_v;
    return value;
}

int load_game_config(const char *path) {
    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, path)) {
        fprintf(stderr, "Config file error at %s:%d - %s\n",
                config_error_file(&cfg),
                config_error_line(&cfg),
                config_error_text(&cfg));
        config_destroy(&cfg);
        return -1;
    }

    GameConfig new_cfg = g_config; // start from defaults

    config_lookup_int(&cfg, "planet_num", &new_cfg.planet_num);
    config_lookup_int(&cfg, "trash_amount", &new_cfg.trash_amount);
    config_lookup_int(&cfg, "window_size", &new_cfg.window_size);
    config_lookup_int(&cfg, "ship_capacity", &new_cfg.ship_capacity);
    config_lookup_int(&cfg, "max_trash_world", &new_cfg.max_trash_world);

    // Enforce compile-time limits to avoid overruns
    new_cfg.planet_num = clamp_int(new_cfg.planet_num, 1, PLANET_NUM);
    new_cfg.trash_amount = clamp_int(new_cfg.trash_amount, 0, N_TRASH);
    new_cfg.window_size = clamp_int(new_cfg.window_size, 100, 5000);
    new_cfg.ship_capacity = clamp_int(new_cfg.ship_capacity, 1, SHIP_CAPACITY);
    new_cfg.max_trash_world = clamp_int(new_cfg.max_trash_world, 1, MAX_TRASH_WORLD);

    g_config = new_cfg;
    config_destroy(&cfg);
    return 0;
}

void init_recycle_index(void) {
    RECYCLE_PLANET_INDEX = rand() % g_config.planet_num;
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
            planets[i].x = rand() % g_config.window_size;
            planets[i].y = rand() % g_config.window_size;
            planets[i].trash_count = 0;
            planets[i].mass = 10.0;
            planets[i].name = 'A'+ i;
            planets[i].is_garbage = 1;
            // initialize planet's default ship
            planets[i].ship.ch = planets[i].name;
            planets[i].ship.position.x = planets[i].x;
            planets[i].ship.position.y = planets[i].y;
            planets[i].ship.velocity.amplitude = 0;
            planets[i].ship.acceleration.amplitude = 0;
            planets[i].ship.velocity.angle = 0;
            planets[i].ship.acceleration.angle = 0;
            planets[i].ship.mass = 1.0;
            planets[i].ship.trash_count = 0;
            planets[i].ship_assigned = 0;
            continue;
        }
        planets[i].x = rand() % g_config.window_size;
        planets[i].y = rand() % g_config.window_size;
        planets[i].trash_count = 0;
        planets[i].mass = 10.0;
        planets[i].name = 'A' + i;
        planets[i].is_garbage= 0;
        planets[i].ship.ch = planets[i].name;
        planets[i].ship.position.x = planets[i].x;
        planets[i].ship.position.y = planets[i].y;
        planets[i].ship.velocity.amplitude = 0;
        planets[i].ship.acceleration.amplitude = 0;
        planets[i].ship.velocity.angle = 0;
        planets[i].ship.acceleration.angle = 0;
        planets[i].ship.mass = 1.0;
        planets[i].ship.trash_count = 0;
        planets[i].ship_assigned = 0;
    }

     for (int i = 0; i < num_planets; i++) {
        for (int j = i + 1; j < num_planets; j++) {
            // Check if planet i and j have the same coordinates
            if (planets[i].x == planets[j].x && planets[i].y == planets[j].y) {
                // If they do, regenerate new coordinates.
                planets[j].x = rand() % g_config.window_size;
                planets[j].y = rand() % g_config.window_size;
                
                // Restart the check for the modified planet j from the beginning
                i = 0; 
                break;
            }
        }
    }
}

void trash_init(Trash_t* trash, int num_trash) {
    for (int i = 0; i < num_trash; i++) {
        trash[i].position.x = rand() % g_config.window_size;
        trash[i].position.y = rand() % g_config.window_size;
        trash[i].velocity.amplitude = 0;
        trash[i].acceleration.amplitude = 0;
        trash[i].velocity.angle = 0;
        trash[i].acceleration.angle = 0;
        trash[i].mass = 1.0;
    }
}

int generate_new_trash(Trash_t* trash, int current_trash, int x, int y){
    if (current_trash >= g_config.max_trash_world) {
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
    