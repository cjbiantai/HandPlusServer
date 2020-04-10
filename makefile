GameServer.out: GameManager.o GameSync.o Server.o main.o Player.o Room.o
	g++ -o GameServer.out GameManager.o GameSync.o Server.o main.o Player.o Room.o -lprotobuf common/game_proto.pb.o
main.o: main.cpp
	g++ -c main.cpp
Player.o: Player.cpp Player.h
	g++ -c Player.cpp Player.h
Room.o: Room.cpp Room.h
	g++ -c Room.cpp Room.h
Server.o: Server.cpp
	g++ -c Server.cpp Server.h
GameManager.o: game_manager.cpp game_manager.h
	g++ -c game_manager.cpp game_manager.h
GameSync.o: game_sync.cpp gamesync.h
	g++ -c game_sync.cpp gamesync.h
clean:
	sudo rm *.o GameServer.out