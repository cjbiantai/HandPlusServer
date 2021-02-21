#include <iostream>
#include "hall_server.h"

int main()
{
    SETTRACELEVEL(log_mask_all);
    hallServer server(16000);
    while(true) {
        server.Work();
    }
    return 0;
}

