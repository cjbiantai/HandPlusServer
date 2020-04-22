GameServer.out: game_sync.o server.o main.o player.o room.o socket_error.o common/game_proto.pb.o
	g++ -g -o GameServer.out game_sync.o server.o main.o player.o room.o socket_error.o -lprotobuf common/game_proto.pb.o
main.o: main.cpp
	g++ -g -c main.cpp
player.o: player.cpp
	g++ -g -c player.cpp
room.o: room.cpp
	g++ -g -c room.cpp
server.o: server.cpp
	g++ -g -c server.cpp
game_sync.o: game_sync.cpp
	g++ -g -c game_sync.cpp
socket_error.o: socket_error.cpp
	g++ -g -c socket_error.cpp
clean:
	sudo rm *.o  GameServer.out
