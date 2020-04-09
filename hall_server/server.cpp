#include <iostream>
#include "hall_server.h"

int main()
{
    SETTRACELEVEL(log_mask_all);
    
    //printf("%d %d\n", c, (unsigned char)c);
    hallServer server;
    while(true) {
        server.Work();
    }
    return 0;
}

