#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "communication.h"
#include "cursor_processing.h"
#include "universe-data.h"
#include "physics-rules.h"
#include "display.h"
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"


//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);


direction_t random_direction() {
    return rand() % 4;
}

int find_ch_info(Ship arr[], int n, char ch) {
    for (int i = 0; i < n; i++)
        if (arr[i].ch == ch)
            return i;
    return -1;
}

void generate_trash_periodically(Trash_t trash[], int* n_trash, Uint32* last_trash_time) {
    Uint32 current_time = SDL_GetTicks();
    
    // Generate new trash every 10 seconds (10000 milliseconds)
    if (current_time - *last_trash_time >= 10000) {
        if (*n_trash < MAX_TRASH_WORLD) {
            int random_x = rand() % WINDOW_SIZE;
            int random_y = rand() % WINDOW_SIZE;
            
            generate_new_trash(trash, *n_trash, random_x, random_y);
            (*n_trash)++;
            
            printf("New trash generated at position (%d, %d). Total trash: %d\n", random_x, random_y, *n_trash);
            fflush(stdout);
        }
        
        *last_trash_time = current_time;
    }
}

void rotate_recycle_planet(Planet_t* planets, Uint32* last_planet_time) {
    Uint32 current_time = SDL_GetTicks();
    
    // Rotate recycle planet every 30 seconds (30000 milliseconds)
    if (current_time - *last_planet_time >= 30000) {
        int old_index = RECYCLE_PLANET_INDEX;
        RECYCLE_PLANET_INDEX = rand() % PLANET_NUM;
        
        planets[old_index].is_garbage = 0;
        
        planets[RECYCLE_PLANET_INDEX].is_garbage = 1;
        
        printf("Recycle planet changed from %c to %c\n", 'A' + old_index, 'A' + RECYCLE_PLANET_INDEX);
        fflush(stdout);
        
        *last_planet_time = current_time;
    }
}

void remove_trash(Trash_t trash[], int *n_trash, int index) {
    if (index < 0 || index >= *n_trash) return;

    // shift all elements left
    for (int j = index; j < *n_trash - 1; j++) {
        trash[j] = trash[j + 1];
    }

    (*n_trash)--;  // one less element
}

void transfer_trash_to_planet(Ship* ship, Planet_t* planet) {
    // move each trash item from ship to planet
    for (int i = 0; i < ship->trash_count; i++) {

        // Do not overflow the planet storage
        if (planet->trash_count < N_TRASH) {
            planet->trash[planet->trash_count] = ship->trash[i];
            planet->trash_count++;
        }
    }

    // Ship loses all trash
    ship->trash_count = 0;
}

void scatter_trash(Ship* ship, Trash_t trash[], int* n_trash, int window_size)
{
    for (int i = 0; i < ship->trash_count; i++) {

        // don't overflow global trash array
        if (*n_trash >= MAX_TRASH_WORLD)
            break;

        // copy the trash
        trash[*n_trash] = ship->trash[i];

        // give it a NEW RANDOM POSITION
        trash[*n_trash].position.x = rand() % window_size;
        trash[*n_trash].position.y = rand() % window_size;

        (*n_trash)++;
    }

    // all trash thrown out -> ship is empty
    ship->trash_count = 0;
}

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        return 1;
    }
    // To get a "random" seed, for the planets' position
    srand(time(NULL));

    init_recycle_index();

    // Initialize planets
    Planet_t planets[PLANET_NUM];
    planets_init(planets, PLANET_NUM);

    // Initialize trash
    Trash_t trash[MAX_TRASH_WORLD];
    trash_init(trash, N_TRASH);

    int ret;
    int n_trash = N_TRASH;      //n_trash is the variable for the dynamic ammount of trash in the world
    int aux_trash_recycle = 0;  //auxiliar variable for printing recycle information
    int aux_trash_spill = 0;    //auxiliar variable for printing spill information
    int aux_ship = 0;           //auxiliar variable for ship recycle warning

    // Initialize display
    SDL_Window* win = disp_init();
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);   
    
    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 186, 20, 255};
    SDL_Color trash_color = {128, 128, 0, 255};
    SDL_Color ship_color = {186, 80, 80, 100};

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        return 1;
    }

    void* fd = create_server_channel();

    Ship char_data[100];
    int n_chars = 0;

    char message_type[100];
    char c;
    direction_t d;

    // Timer for trash generation
    Uint32 last_trash_time = SDL_GetTicks();
    // Timer for recycle planet rotation
    Uint32 last_planet_time = SDL_GetTicks();

    int close = 0;
    while(close == 0){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                close = 1;
            }
        }
        
        planet_drawer(planets, PLANET_NUM, rend, planet_color, garbage_planet_color, font);   
        trash_drawer(trash, n_trash, rend, trash_color);
        // Draw all ships
        ship_drawer(char_data, n_chars, rend, ship_color, font);

        if (n_chars > 0){
            // Generate new trash every 10 seconds
            generate_trash_periodically(trash, &n_trash, &last_trash_time);
        }
        
        // Rotate recycle planet every 30 seconds
        rotate_recycle_planet(planets, &last_planet_time);

        message_type[0] = '\0';  // initialize to empty
        read_message(fd, message_type, &c, &d);

        if (strcmp(message_type, "CONNECT") == 0) {

            // assign next available planet ship to this client
            int assigned = -1;
            for (int p = 0; p < PLANET_NUM; p++) {
                if (!planets[p].ship_assigned) {
                    assigned = p;
                    break;
                }
            }
            if (assigned == -1) {
                send_response(fd, "NOT OK");
                continue;
            }

            // assign the planet's ship to a new client
            char assigned_char = planets[assigned].ship.ch;
            // create client ship
            char_data[n_chars].ch = assigned_char;
            char_data[n_chars].position.x = planets[assigned].x;
            char_data[n_chars].position.y = planets[assigned].y;
            char_data[n_chars].velocity.amplitude = 0;
            char_data[n_chars].velocity.angle = 0;
            char_data[n_chars].acceleration.amplitude = 0;
            char_data[n_chars].acceleration.angle = 0;
            char_data[n_chars].mass = 1.0;
            char_data[n_chars].trash_count = 0;
            // mark planet ship as assigned
            planets[assigned].ship_assigned = 1;
            n_chars++;

            // reply with the assigned character so client knows its ship
            char resp[4] = {assigned_char, '\0', '\0', '\0'};
            send_response(fd, resp);

        } else if (message_type[0] != '\0' && strcmp(message_type, "MOVE") == 0) {            int pos = find_ch_info(char_data, n_chars, c);
            if (pos != -1) {
                // Apply thrust to the ship's velocity instead of teleporting position
                new_position(&char_data[pos], d);
                send_response(fd, "OK");
            }
        }

        



        
        // Trash interaction
        for (int i = 0; i < n_chars; i++){
            for (int j = 0; j < n_trash; j++){

                float dx = (char_data[i].position.x ) - trash[j].position.x;
                float dy = (char_data[i].position.y ) - trash[j].position.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                    // store trash in ship if not full
                    if (char_data[i].trash_count < SHIP_CAPACITY) {
                        char_data[i].trash[char_data[i].trash_count] = trash[j];
                        char_data[i].trash_count++;
                        remove_trash(trash, &n_trash, j);
                        j--;  // adjust index since we removed an element
                    } else {
                        // ship full: cannot pick more
                    }
                    aux_trash_spill = n_trash;
                    aux_trash_recycle = N_TRASH - n_trash;
                
                    if (char_data[i].trash_count == SHIP_CAPACITY  && aux_ship == 0){
                        printf("Ship %c is full with %d pieces of trash! Please go to recycling planet\n", char_data[i].ch, char_data[i].trash_count);
                        fflush(stdout);
                        aux_ship =1;
                    }else{
                        if (char_data[i].trash_count < SHIP_CAPACITY){
                            printf("Amount of trash in client %c: %d\n", char_data[i].ch, char_data[i].trash_count);
                            fflush(stdout);
                            aux_ship =0;
                        }
                    }
                
                }
            }
        }

        // Planet interaction
        for (int i = 0; i < n_chars; i++){
            for (int j = 0; j < PLANET_NUM; j++){
                float dx = (char_data[i].position.x ) - planets[j].x;
                float dy = (char_data[i].position.y ) - planets[j].y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                    if (j == RECYCLE_PLANET_INDEX) {
                        // move trash from ship to planet
                        transfer_trash_to_planet(&char_data[i], &planets[j]);
                        
                        if (aux_trash_recycle == planets[j].trash_count){
                            printf("Ship %c delivered trash to planet %c! Planet now has %d pieces.\n",
                                char_data[i].ch, planets[j].name, planets[j].trash_count);
                            aux_trash_recycle = 0;
                        }
                    } else {
                        // ship crashes into other planet, scatter trash                       
                        scatter_trash(&char_data[i], trash, &n_trash, WINDOW_SIZE);

                        if (aux_trash_spill != n_trash){
                            printf("Ship %c crashed into %c and scattered its trash!\n",
                                char_data[i].ch, planets[j].name);
                            aux_trash_spill = n_trash;    
                        }
                    }
                }
            }
        }

        for(int i = 0; i < n_trash; i++){
            for(int j = 0; j < PLANET_NUM; j++){
                if (n_chars > 0){
                    if(trash_planet_collision(&trash[i],&planets[j]) == 1){
                        ret = generate_new_trash(trash, n_trash, planets[j].x, planets[j].y);
                        if (ret == -1) {
                            printf("Ending game.\n");
                            end_game(rend, win);
                            close = 1;
                        }
                        else{
                            n_trash++;
                            printf("Trash counter incremented! Current trash count: %d\n", n_trash);
                        }
                        
                    }
                }
            }   
        }
        
        new_trash_acceleration(planets, PLANET_NUM, trash, n_trash);
        new_trash_velocity(trash, n_trash);
        new_trash_position(trash, n_trash);

        
        new_ship_acceleration(planets, PLANET_NUM, char_data, n_chars);
        new_ship_velocity(char_data, n_chars);
        new_ship_position(char_data, n_chars);
        
        SDL_RenderPresent(rend);

        SDL_Delay(10);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}