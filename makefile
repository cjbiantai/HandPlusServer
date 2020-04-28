GameServer.out: common/all.h game_sync.o server.o main.o player.o room.o socket_error.o socket_connect.o hall_sync.o common/game_proto.pb.o
	g++ -std=c++11 -g -o GameServer.out game_sync.o server.o main.o player.o room.o socket_error.o socket_connect.o hall_sync.o -lprotobuf common/game_proto.pb.o
main.o: common/all.h main.cpp
	g++ -std=c++11 -g -c main.cpp
player.o:common/all.h  player.cpp
	g++ -std=c++11 -g -c player.cpp
room.o: common/all.h room.cpp
	g++ -std=c++11 -g -c room.cpp
server.o: common/all.h server.cpp
	g++ -std=c++11 -g -c server.cpp
game_sync.o: common/all.h game_sync.cpp
	g++ -std=c++11 -g -c game_sync.cpp
socket_error.o: common/all.h socket_error.cpp
	g++ -std=c++11 -g -c socket_error.cpp
socket_connect.o: common/all.h socket_connect.cpp
	g++ -std=c++11 -g -c socket_connect.cpp
hall_sync.o: common/all.h hall_sync.cpp
	g++ -std=c++11 -g -c hall_sync.cpp
clean:
	sudo rm *.o  GameServer.out
