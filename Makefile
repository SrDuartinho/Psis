CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Object files for main program (now using universe-simulator)
OBJ = physics-rules.o universe-data.o display.o

all: new-trash-ship-client new-universe-server

test: new-universe-server new-trash-ship-client

#
# --- Main program (universe-simulator) ---
#
program: $(OBJ)
	$(CC) $(OBJ) -lm -o program -lSDL2 -lSDL2_ttf -lSDL2_gfx -lm


PROTO_SRC = proto/messages.proto

proto/messages.pb.cc proto/messages.pb.h: $(PROTO_SRC)
	protoc --cpp_out=proto $(PROTO_SRC)

physics-rules.o: physics-rules.c physics-rules.h universe-data.h
	$(CC) $(CFLAGS) -c physics-rules.c

universe-data.o: universe-data.c universe-data.h
	$(CC) $(CFLAGS) -c universe-data.c


display.o: display.c display.h universe-data.h
	$(CC) $(CFLAGS) -c display.c

#
# --- Server and Client ---
#

new-universe-server: proto/messages.pb.cc new-universe-server.c cursor_processing.c physics-rules.c universe-data.o communication_proto.cpp communication.h cursor_processing.h physics-rules.h display.c display.h
	$(CC) $(CFLAGS) -o new-universe-server new-universe-server.c cursor_processing.c physics-rules.c communication_proto.cpp proto/messages.pb.cc display.c universe-data.o \
		-lncurses -lzmq -lSDL2 -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lprotobuf -lstdc++ -lm


new-trash-ship-client: proto/messages.pb.cc new-trash-ship-client.c universe-data.o communication_proto.cpp communication.h
	$(CC) $(CFLAGS) -o new-trash-ship-client new-trash-ship-client.c communication_proto.cpp proto/messages.pb.cc universe-data.o \
		-lncurses -lzmq -lprotobuf -lstdc++ -lm -lSDL2 -lSDL2_image


clean:
	rm -f *.o program new-trash-ship-client new-universe-client new-universe-server