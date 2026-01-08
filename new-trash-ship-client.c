#include <ncurses.h>
#include <ctype.h> 
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "SDL2/SDL_pixels.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "display.h"
#include "communication.h"
#include "universe-data.h"

// Global variables to control thread and window state
int thread_running = 1;
int window_close = 0;

void initialize_screen(){
	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */
}

/**
 * Function that recieves game state from server, and displays it.
 * Should be run in a separate thread.
 * @param fd: file descriptor of the socket to be used to receive game state from the server (socket from create_client_channel())
 */
void* info_receiver(void * fd){

    Ship ship[100];
    int n_ships = 0;
    Planet_t planets[PLANET_NUM];
    int n_planets = 0;
    Trash_t trash[MAX_TRASH_WORLD];
    int n_trash = 0;

    int client_server = 1; //1 for client, 0 for server
    SDL_Window* win = disp_init(client_server);
    SDL_Renderer* rend = rend_init(win);
    SDL_RenderPresent(rend);

    // initialize TTF and font for labels
    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        return NULL;
    }
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);
    if (!font) {
        printf("Font error: %s\n", TTF_GetError());
        // continue without labels
        font = NULL;
    }

    SDL_Color planet_color = {80, 80, 186, 255};
    SDL_Color garbage_planet_color = {20, 186, 186, 255};
    SDL_Color trash_color = {128, 128, 0, 255};
    SDL_Color ship_color = {186, 80, 80, 100};

    int close = 0;
    int i;

    while (close == 0){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                window_close = 1;
                close = 1;
                break;
            }
        }

        if(window_close == 1 || close == 1){    // To avoid further processing after game end
            close = 1;
            break;
        }

        if(receive_game_state(fd, ship, &n_ships, planets, &n_planets, trash, &n_trash)){
            if(n_ships == 0 && n_planets == 0){ //Game ended
                printf("Game ended by server.\n");
                close = 1;
                break;
            }
            SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
            SDL_RenderClear(rend);
            planet_drawer(planets, n_planets, rend, planet_color, garbage_planet_color, font);
            trash_drawer(trash, MAX_TRASH_WORLD, rend, trash_color);
            for(i = 0; i < n_ships; i++){
                draw_char(rend, font, ship[i].ch, ship[i].position.x, ship[i].position.y, ship_color, ship[i].trash_count);
            }
            SDL_RenderPresent(rend);
            SDL_Delay(10);
        }
    }

    if (font) {
        TTF_CloseFont(font);
        TTF_Quit();
    }
    
    end_game(rend, win);
  	endwin();			/* End curses mode		  */

    thread_running = 0;
    return NULL;
}

int main(){

    if (load_game_config("libconfig/universe.conf") != 0) {
        printf("Using default configuration (could not load config file).\n");
    } else {
        printf("Loaded configuration from libconfig/universe.conf.\n");
    }

    // Using two sockets to communicate with the server
    // One socket for sending movement commands, and receiving a short response (REQ/REP)
    // Another for receiving game state, and sending a short acknowledgment (PUB/SUB)
    void * movement_fd = create_client_channel("127.0.0.1");
    // The SUB socket for receiving game state made more sense than PUB socket, as the client only receives data
    void* context = zmq_ctx_new();
    void* state_fd = zmq_socket (context, ZMQ_SUB);
    zmq_connect(state_fd, "tcp://127.0.0.1:5556");
    zmq_setsockopt(state_fd, ZMQ_SUBSCRIBE, "", 0); //Subscribe to all messages

    char ch = '\0';
    // ask server to assign the next available ship
    send_connection_message(movement_fd, ' ');
    char message[100];
    receive_response (movement_fd, message);
    if (strcmp(message, "NOT OK") == 0){
        fprintf(stderr, "No ships available on server\n");
        exit(-1);
    }
    // server returns assigned character as first byte
    if (message[0] != '\0') ch = message[0];
    else { fprintf(stderr, "Invalid assignment from server\n"); exit(-1); }

    int n = 0;
    int key;
    direction_t direction;
    int running = 1;

    initialize_screen();

    pthread_t info_thread;
    pthread_create(&info_thread, NULL, (void *)info_receiver, state_fd);
    
    // Set non-blocking mode for input
    timeout(10);

    do
    {
        // Check if info_receiver thread is still running
        if (thread_running == 0) {
            // Thread has finished, exit main loop
            running = 0;
            break;
        }
        
        key = getch();			/* Wait for user input */

        n++;
        switch (key)
        {
        case KEY_LEFT:
            mvprintw(0,0,"%d Left arrow is pressed", n);
            direction = LEFT;
            break;
        case KEY_RIGHT:
            mvprintw(0,0,"%d Right arrow is pressed", n);
            direction = RIGHT;
            break;
        case KEY_DOWN:
            mvprintw(0,0,"%d Down arrow is pressed", n);
            direction = DOWN;
            break;
        case KEY_UP:
            mvprintw(0,0,"%d :Up arrow is pressed", n);
            direction = UP;
            break;
        case 'q':
            mvprintw(0,0,"%d :Quit key is pressed", n);
            direction = QUIT;
            running = 0;
            window_close = 1;
            break;

        default:
            key = 'x'; 
            break;
        }

        if(running == 0){
            break;
        }
        if (key != 'x'){
            send_movement_message(movement_fd, ch, direction);
            char message[100];
            receive_response(movement_fd, message);
            if (strcmp(message, "WALL") ==0){
                mvprintw(1,0,"You hit a wall!");
                break;
            }
        }
        
        refresh();			/* Print it on to the real screen */
        
        SDL_Delay(10);
        
    }while(key != 27 && running && !window_close);
    

    send_movement_message(movement_fd, ch, QUIT);
    char final_message[100];
    receive_response(movement_fd, final_message);
    printf("User has quit. Answer from the server: %s\n", final_message);
    refresh();
    
    // Try to join the thread if it hasn't been joined yet
    pthread_join(info_thread, NULL);
	return 0;
}