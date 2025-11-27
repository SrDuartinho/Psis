all: universe-simulator universe-client universe-server

universe-simulator: universe-simulator.c
	gcc -o universe-simulator universe-simulator.c -lSDL2

universe-client: universe-client.c
	gcc -o universe-client universe-client.c

universe-server: universe-server.c
	gcc -o universe-server universe-server.c