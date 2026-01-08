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
#include <pthread.h>


//Function prototypes
void planets_init(Planet_t* planets, int num_planets);
SDL_Color random_color();
Uint32 SDL_ColorToUint(SDL_Color c);

typedef struct{
    Ship* ships;
    int* n_ships;
    Planet_t* planets;
    Trash_t* trash;
    int* n_trash;
    pthread_mutex_t* mutex;
    void *state_fd;
}ThreadData_t;

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
        if (*n_trash < g_config.max_trash_world) {
            int random_x = rand() % g_config.window_size;
            int random_y = rand() % g_config.window_size;
            
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
        RECYCLE_PLANET_INDEX = rand() % g_config.planet_num;
        
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
        if (*n_trash >= g_config.max_trash_world)
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


/** 
 * @brief Writes the current game statistics to "dashboard.txt".
 * @param planets Array of planets.
 * @param num_planets Number of planets.
 * @param ships Array of ships.
 * @param n_ships Number of ships.
 * @param total_trash The current amount of trash in the world.
 */

 /**
 * @brief Removes a ship from the ships array and marks its corresponding planet's ship as unassigned.
 * @param ships Array of ships.
 * @param n_ships Pointer to the number of ships.
 * @param index Index of the ship to be removed.
 * @param planets Array of planets.
 */

void remove_ship(Ship ships[], int* n_ships, int index, Planet_t planets[]) {
    if (index < 0 || index >= *n_ships) return;

    // Find the corresponding planet and mark its ship as unassigned
    for (int i = 0; i < g_config.planet_num; i++) {
        if (planets[i].ship.ch == ships[index].ch) {
            planets[i].ship_assigned = 0;
            break;
        }
    }

    // Shift all elements left
    for (int j = index; j < *n_ships - 1; j++) {
        ships[j] = ships[j + 1];
    }

    (*n_ships)--; // one less ship
}

void statistics_writer(Planet_t planets[], int num_planets, Ship ships[], int n_ships,
    int total_trash) {
    FILE *f;
    f = fopen("dashboard.txt", "w");

    if (f == NULL) {
        perror("Error opening dashboard.txt");
        return;
    }

    if(planets == NULL && ships == NULL){
        fprintf(f, "END\n");
        fclose(f);
        return;
    }

    for(int i = 0; i < num_planets; i++) {
        fprintf(f, "%c - %d\n", planets[i].name, planets[i].trash_count);
    }

    fprintf(f, "----\n");

    for(int i = 0; i < n_ships; i++) {
        fprintf(f, "%c - %d\n", ships[i].ch, ships[i].trash_count);
    }

    fprintf(f, "****\n");
    fprintf(f, "%d %d", total_trash, g_config.max_trash_world);
    fclose(f);
}

/**
 * @brief Handler function for client connections. Receives and processes the keystrokes 
 * sent by the clients. Creates its own socket REQ/REP.Should be run in a separate thread for each client.
 * @param: void arg* a ThreadData_t struct containing all necessary data for the thread to operate.
 * @return: NULL
 */
void* client_handler(void* arg) {
    //Each client has its own socket
    void *command_fd = create_server_channel();
    int timeout = 10;  // 10ms timeout to avoid blocking
    zmq_setsockopt(command_fd, ZMQ_RCVTIMEO, &timeout, sizeof(int)); //10ms timeout to avoid blocking
    
    //Unpack thread data
    ThreadData_t* data = (ThreadData_t*)arg;
    Ship* ships = data->ships;
    int* n_ships = data->n_ships;
    Planet_t* planets = data->planets;
    Trash_t* trash = data->trash;
    int* n_trash = data->n_trash;
    pthread_mutex_t* mutex = data->mutex;
    void* state_fd = data->state_fd;

    char message_type[100];
    char c = '\0';
    direction_t d;

    while (1) {
        message_type[0] = '\0';  // initialize to empty
        read_message(command_fd, message_type, &c, &d);
        if(strcmp(message_type, "CONNECT") == 0) {
            // Handle connection request
        } 
        else if(strcmp(message_type, "MOVE") == 0) {
            if(d == 'q'){
            //
            }
            // Lock mutex before accessing shared data
            // pthread_mutex_lock(mutex);
        }
    }
    return NULL;
}


int main() {

    if (load_game_config("libconfig/universe.conf") != 0) {
        printf("Using default configuration (could not load config file).\n");
    } else {
        printf("Loaded configuration from libconfig/universe.conf.\n");
    }

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
    planets_init(planets, g_config.planet_num);

    // Initialize trash
    Trash_t trash[MAX_TRASH_WORLD];
    trash_init(trash, g_config.trash_amount);

    int ret;
    int n_trash = g_config.trash_amount;      //n_trash is the variable for the dynamic ammount of trash in the world
    int aux_trash_recycle = 0;  //auxiliar variable for printing recycle information
    int aux_trash_spill = 0;    //auxiliar variable for printing spill information
    int aux_ship = 0;           //auxiliar variable for ship recycle warning

    // Initialize display
    SDL_Window* win = disp_init(0); //0 for server
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);   
    
    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 186, 20, 255};
    SDL_Color trash_color = {128, 128, 0, 255};
    SDL_Color ship_color = {186, 80, 80, 100};

    // initialize TTF and font for labels
    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        return 1;
    }
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        // continue without labels
        font = NULL;
    }

    void* command_fd = create_server_channel();
    int timeout = 10;  // 10ms timeout to avoid blocking
    zmq_setsockopt(command_fd, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    void* context = zmq_ctx_new();
    void* state_fd = zmq_socket(context, ZMQ_PUB);
    zmq_bind(state_fd, "tcp://*:5556"); //Allow local and remote connections

    Ship ships[100];
    int n_ships = 0;

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
                send_game_state(state_fd, NULL, 0, NULL, 0, NULL, 0); // Indicate game end to clients
                close = 1;
            }
        }
        
        planet_drawer(planets, g_config.planet_num, rend, planet_color, garbage_planet_color, font);   
        trash_drawer(trash, n_trash, rend, trash_color);
        // Draw all ships
        ship_drawer(ships, n_ships, rend, ship_color, font);

        if (n_ships > 0){
            // Generate new trash every 10 seconds
            generate_trash_periodically(trash, &n_trash, &last_trash_time);
        }
        
        // Rotate recycle planet every 30 seconds
        rotate_recycle_planet(planets, &last_planet_time);

        message_type[0] = '\0';  // initialize to empty
        read_message(command_fd, message_type, &c, &d);

        if (strcmp(message_type, "CONNECT") == 0) {

            // assign next available planet ship to this client
            int assigned = -1;
            for (int p = 0; p < g_config.planet_num; p++) {
                if (!planets[p].ship_assigned) {
                    assigned = p;
                    break;
                }
            }
            if (assigned == -1) {
                send_response(command_fd, "NOT OK");
                continue;
            }

            // assign the planet's ship to a new client
            char assigned_char = planets[assigned].ship.ch;
            // create client ship
            ships[n_ships].ch = assigned_char;
            ships[n_ships].position.x = planets[assigned].x;
            ships[n_ships].position.y = planets[assigned].y;
            ships[n_ships].velocity.amplitude = 0;
            ships[n_ships].velocity.angle = 0;
            ships[n_ships].acceleration.amplitude = 0;
            ships[n_ships].acceleration.angle = 0;
            ships[n_ships].mass = 1.0;
            ships[n_ships].trash_count = 0;
            // mark planet ship as assigned
            planets[assigned].ship_assigned = 1;
            n_ships++;

            // reply with the assigned character so client knows its ship
            char resp[4] = {assigned_char, '\0', '\0', '\0'};
            send_response(command_fd, resp);

        } else if (message_type[0] != '\0' && strcmp(message_type, "MOVE") == 0) {            int pos = find_ch_info(ships, n_ships, c);
            if (d == 'q'){
                // remove ship from array
                remove_ship(ships, &n_ships, pos, planets);
                send_response(command_fd, "OK");
                printf("Ship %c has quit the game.\n", c);
            }
            if (pos != -1) {
                // Apply thrust to the ship's velocity instead of teleporting position
                new_position(&ships[pos], d);
                send_response(command_fd, "OK");
            }
        }

    
        // Trash interaction
        for (int i = 0; i < n_ships; i++){
            for (int j = 0; j < n_trash; j++){

                float dx = (ships[i].position.x ) - trash[j].position.x;
                float dy = (ships[i].position.y ) - trash[j].position.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                    // store trash in ship if not full
                    if (ships[i].trash_count < g_config.ship_capacity) {
                        ships[i].trash[ships[i].trash_count] = trash[j];
                        ships[i].trash_count++;
                        remove_trash(trash, &n_trash, j);
                        j--;  // adjust index since we removed an element
                    } else {
                        // ship full: cannot pick more
                    }
                    aux_trash_spill = n_trash;
                    aux_trash_recycle = g_config.max_trash_world - n_trash;
                
                    if (ships[i].trash_count == g_config.ship_capacity  && aux_ship == 0){
                        printf("Ship %c is full with %d pieces of trash! Please go to recycling planet\n", ships[i].ch, ships[i].trash_count);
                        fflush(stdout);
                        aux_ship =1;
                    }else{
                        if (ships[i].trash_count < g_config.ship_capacity){
                            printf("Amount of trash in client %c: %d\n", ships[i].ch, ships[i].trash_count);
                            fflush(stdout);
                            aux_ship =0;
                        }
                    }
                
                }
            }
        }

        // Planet interaction
        for (int i = 0; i < n_ships; i++){
            for (int j = 0; j < g_config.planet_num; j++){
                float dx = (ships[i].position.x ) - planets[j].x;
                float dy = (ships[i].position.y ) - planets[j].y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance <= 20.0f) {  // within radius of 20
                    if (j == RECYCLE_PLANET_INDEX) {
                        // move trash from ship to planet
                        transfer_trash_to_planet(&ships[i], &planets[j]);
                        
                        if (aux_trash_recycle == planets[j].trash_count){
                            printf("Ship %c delivered trash to planet %c! Planet now has %d pieces.\n",
                                ships[i].ch, planets[j].name, planets[j].trash_count);
                            aux_trash_recycle = 0;
                        }
                    } else {
                        // ship crashes into other planet, scatter trash                       
                        scatter_trash(&ships[i], trash, &n_trash, g_config.window_size);

                        if (aux_trash_spill != n_trash){
                            printf("Ship %c crashed into %c and scattered its trash!\n",
                                ships[i].ch, planets[j].name);
                            aux_trash_spill = n_trash;    
                        }
                    }
                }
            }
        }

        for(int i = 0; i < n_trash; i++){
            for(int j = 0; j < g_config.planet_num; j++){
                if (n_ships > 0){
                    if(trash_planet_collision(&trash[i],&planets[j]) == 1){
                        ret = generate_new_trash(trash, n_trash, planets[j].x, planets[j].y);
                        if (ret == -1) {
                            printf("Ending game.\n");
                            send_game_state(state_fd, NULL, 0, NULL, 0, NULL, 0); //Notify clients of the game endinf
                            statistics_writer(NULL, 0, NULL, 0, 0);
                            end_game(rend, win);
                            close = 1;
                            break;  
                        }
                        else{
                            n_trash++;
                            printf("Trash counter incremented! Current trash count: %d\n", n_trash);
                        }
                        
                    }
                }
                if (close == 1){    //Exit nested loops
                    break;
                }   
            }
        }

        if (close == 1){    // To avoid further processing after game end
            break;
        }
        new_trash_acceleration(planets, g_config.planet_num, trash, n_trash);
        new_trash_velocity(trash, n_trash);
        new_trash_position(trash, n_trash);

        
        new_ship_acceleration(planets, g_config.planet_num, ships, n_ships);
        new_ship_velocity(ships, n_ships);
        new_ship_position(ships, n_ships);

        statistics_writer(planets, PLANET_NUM, ships, n_ships, n_trash);
        
        // Broadcast game state to all clients
        send_game_state(state_fd, ships, n_ships, planets, g_config.planet_num, trash, n_trash);
        
        SDL_RenderPresent(rend);

        SDL_Delay(10);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}