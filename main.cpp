#include "server_sync.h"
#include "hall_sync.h"
#include "game_sync.h"
#include "server.h"

#define GAMESERVER

int main(){
#ifdef HALLSERVER
	ServerSync sync=HallSync();
#endif
#ifdef GAMESERVER
	ServerSync sync=GameSync();
#endif
    //SETTRACELEVEL(log_mask_all);
    Server server(233,sync);
    while(true) {
        server.WorkOnce();
    }
    return 0;
}

