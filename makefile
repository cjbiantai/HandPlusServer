GameServer.out: game_sync.o server.o main.o player.o room.o common/game_proto.pb.o
	g++ -o GameServer.out game_sync.o server.o main.o player.o room.o -lprotobuf common/game_proto.pb.o
main.o: main.cpp
	g++ -c main.cpp
player.o: player.cpp
	g++ -c player.cpp
room.o: room.cpp
	g++ -c room.cpp
server.o: server.cpp
	g++ -c server.cpp
game_sync.o: game_sync.cpp
	g++ -c game_sync.cpp
clean:
	sudo rm *.o  GameServer.out