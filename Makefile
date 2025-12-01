CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Object files for main program (now using universe-simulator)
OBJ = universe-simulator.o gravitation.o universe-data.o

all: program universe-client universe-server

#
# --- Main program (universe-simulator) ---
#
program: $(OBJ)
	$(CC) $(OBJ) -lm -o program -lSDL2 -lSDL2_gfx -lm

universe-simulator.o: universe-simulator.c universe-data.h gravitation.h
	$(CC) $(CFLAGS) -c universe-simulator.c

gravitation.o: gravitation.c gravitation.h universe-data.h
	$(CC) $(CFLAGS) -c gravitation.c

universe-data.o: universe-data.c universe-data.h
	$(CC) $(CFLAGS) -c universe-data.c


#
# --- Universe Client ---
#
universe-client: universe-client.o universe-data.o gravitation.o
	$(CC) universe-client.o universe-data.o gravitation.o -lm -o universe-client

universe-client.o: universe-client.c universe-data.h gravitation.h
	$(CC) $(CFLAGS) -c universe-client.c


#
# --- Universe Server ---
#
universe-server: universe-server.o universe-data.o gravitation.o
	$(CC) universe-server.o universe-data.o gravitation.o -lm -o universe-server

universe-server.o: universe-server.c universe-data.h gravitation.h
	$(CC) $(CFLAGS) -c universe-server.c


clean:
	rm -f *.o program universe-client universe-server
