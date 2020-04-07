Server:*.cpp ../common/log/*.cpp ../socket/*.cpp ../common/game_proto.proto ../common/mysql/mysql.cpp
	protoc -I=../common/ --cpp_out=../common/ ../common/game_proto.proto 
	g++ -g -std=c++11 *.cpp ../common/log/*.cpp ../socket/*.cpp ../common/game_proto.pb.cc ../common/mysql/mysql.cpp `mysql_config --cflags --libs` -o server -lprotobuf

.PHONY clean:
	rm -rf *.o
