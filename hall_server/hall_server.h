#pragma once
#include "../socket/socket.h"
#include "../common/consts.h"
#include "process_log.h"

class hallServer : socketBase {
public:
    hallServer(int);
    ~hallServer(){}
    void Work();

private:

};

