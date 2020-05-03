#include"common/log_manager.h"
#include"socket_error.h"
#include "server_sync.h"
#include "game_sync.h"
#include "server.h"

#define GAMESERVER

int main(int argc,char **argv){
	if(argc<2)
		return 0*printf("input port\n");
    LogManager::Init(0,DEBUG_MODE_STDOUT|DEBUG_MODE_FILE);
    //LogManager::Init(0,DEBUG_MODE_STDOUT);
    srand(time(0));
#ifdef HALLSERVER
	ServerSync *sync=new HallSync();
#endif
#ifdef GAMESERVER
	ServerSync *sync=new GameSync();
#endif
    SocketError::sync=sync;
    Server server(atoi(argv[1]),sync);
    while(true) {
        server.WorkOnce();
    }
    delete sync;
    LogManager::Close();
    return 0;
}

