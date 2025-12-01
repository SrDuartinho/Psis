all: universe-simulator universe-client universe-server

universe-simulator: universe-simulator.c
	gcc -o universe-simulator universe-simulator.c -lSDL2 -lSDL2_gfx -lm

universe-client: universe-client.c
	gcc -o universe-client universe-client.c

universe-server: universe-server.c
	gcc -o universe-server universe-server.c