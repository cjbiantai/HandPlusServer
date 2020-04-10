GameServer.out: GameSync.o Server.o main.o Player.o Room.o game_proto.pb.o
	g++ -o GameServer.out GameSync.o Server.o main.o Player.o Room.o -lprotobuf game_proto.pb.o
main.o: main.cpp
	g++ -c main.cpp
Player.o: Player.cpp
	g++ -c Player.cpp
Room.o: Room.cpp
	g++ -c Room.cpp
Server.o: Server.cpp
	g++ -c Server.cpp
GameSync.o: game_sync.cpp
	g++ -c game_sync.cpp
game_proto.pb.o: common/game_proto.proto
	protoc --cpp_out=.. common/game_proto.proto
	g++ -c game_proto.pb.cc -lprotobuf
clean:
	sudo rm *.o game_proto.pb.*  GameServer.out