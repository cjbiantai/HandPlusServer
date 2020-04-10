#include "server_sync/server_sync.h"
#include "server_sync/hall_sync.h"
#include "server_sync/game_sync.h"
#include "common/process_log.h"
#include "server.h"

#define HALLSERVER

int main(){
#ifdef HALLSERVER
	ServerSync sync=HallSync();
#endif
#ifdef GAMESERVER
	ServerSync sync=GameSync();
#endif
    SETTRACELEVEL(log_mask_all);
    Server server(16000,sync);
    while(true) {
        server.WorkOnce();
    }
    return 0;
}

