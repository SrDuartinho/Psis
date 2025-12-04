CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Object files for main program (now using universe-simulator)
OBJ = universe-simulator.o gravitation.o universe-data.o display.o

all: program universe-client universe-server universe-simulator

test: universe-server universe-client

#
# --- Main program (universe-simulator) ---
#
program: $(OBJ)
	$(CC) $(OBJ) -lm -o program -lSDL2 -lSDL2_ttf -lSDL2_gfx -lm


PROTO_SRC = messages.proto

messages.pb.cc messages.pb.h: $(PROTO_SRC)
	protoc --cpp_out=. $(PROTO_SRC)

gravitation.o: gravitation.c gravitation.h universe-data.h
	$(CC) $(CFLAGS) -c gravitation.c

universe-data.o: universe-data.c universe-data.h
	$(CC) $(CFLAGS) -c universe-data.c

universe-simulator.o: universe-simulator.c universe-data.h gravitation.h
	$(CC) $(CFLAGS) -c universe-simulator.c	

display.o: display.c display.h universe-data.h
	$(CC) $(CFLAGS) -c display.c

#
# --- Server and Client ---
#

universe-server: messages.pb.cc universe-server.c universe-data.o communication_proto.cpp communication.h gravitation.c gravitation.h display.c display.h
	$(CC) $(CFLAGS) -o universe-server universe-server.c communication_proto.cpp messages.pb.cc gravitation.c display.c universe-data.o \
		-lncurses -lzmq -lSDL2 -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lprotobuf -lstdc++ -lm


universe-client: messages.pb.cc universe-client.c universe-data.o communication_proto.cpp communication.h
	$(CC) $(CFLAGS) -o universe-client universe-client.c communication_proto.cpp messages.pb.cc universe-data.o \
		-lncurses -lzmq -lprotobuf -lstdc++ -lm -lSDL2 -lSDL2_image

universe-simulator: universe-simulator.o gravitation.o universe-data.o display.o
	$(CC) $(CFLAGS) -o universe-simulator universe-simulator.o gravitation.o universe-data.o display.o -lSDL2 -lSDL2_ttf -lSDL2_gfx -lprotobuf -lstdc++ -lm

clean:
	rm -f *.o program universe-client universe-server universe-simulator