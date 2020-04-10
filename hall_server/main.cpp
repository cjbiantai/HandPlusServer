#include "hall_server.h"
#include "../common/heads.h"
#include "config.h"

#define CONFIG_PATH "./config/config.xml"
Config config;

bool LoadConfig() {
    if(config.LoadConfig(CONFIG_PATH)) {
        printf("success loadconfig!\n");
        return true;
    }else {
        printf("faild to loadconfig!\n");
        return false;
    }
}
int main()
{
    LoadConfig();

    SETTRACELEVEL(log_mask_all);
    
    //printf("%d %d\n", c, (unsigned char)c);
    hallServer server(config);
    while(true) {
        server.Work();
    }
    return 0;
}

