#include"socket_error.h"
#include "server_sync.h"
//#include "hall_sync.h"
#include "game_sync.h"
#include "server.h"

#define GAMESERVER

int main(int argc,char **argv){
	if(argc<2)
		return 0*printf("input port\n");
#ifdef HALLSERVER
	ServerSync *sync=new HallSync();
#endif
#ifdef GAMESERVER
	ServerSync *sync=new GameSync();
#endif
    //SETTRACELEVEL(log_mask_all);
    SocketError::sync=sync;
    Server server(atoi(argv[1]),sync);
    while(true) {
        server.WorkOnce();
    }
    delete sync;
    return 0;
}

