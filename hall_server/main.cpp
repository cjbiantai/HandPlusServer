#include "hall_server.h"
#include "../common/heads.h"
#include "config.h"
#include "process_log.h"

#define CONFIG_PATH "./config/config.xml"
Config config;

void InitDaemon(int8_t nochdir, int8_t noclose) {
        daemon(nochdir, noclose);
        signal(SIGINT,  SIG_IGN);
        signal(SIGHUP,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
}

bool LoadConfig() {
    if(config.LoadConfig(CONFIG_PATH)) {
        printf("success loadconfig!\n");
        return true;
    }else {
        printf("faild to loadconfig!\n");
        return false;
    }
}

int main(int argc, char** argv) {
    SETTRACELEVEL(-1);
    bool is_daemon = false;
    if(1 < argc && !strcasecmp(argv[1], "-d" ) )
    {
        is_daemon = true;
    }else {
        is_daemon = false;
    }
    if (is_daemon) {
        InitDaemon(1,0);
    }
    LoadConfig();
    //printf("%d %d\n", c, (unsigned char)c);
    hallServer server(config);
    while(true) {
        server.Work();
    }
    return 0;
}

