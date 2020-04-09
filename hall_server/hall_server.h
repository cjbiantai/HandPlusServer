#pragma once
#include "../server_base/server_base.h"
#include "../common/consts.h"
#include "../common/mysql/mysql.h"
#include "process_log.h"
#include "../common/game_proto.pb.h"
#include "../common/RecvDataManager.h"

class hallServer : serverBase {
public:
    hallServer();
    ~hallServer(){}
    void HandleEvent(int, int);
    void HandleClose(int);
    void HandleLogIn(GameProto::ClientMsg, int);
    void HandleRegist(GameProto::ClientMsg, int);
    void HandleSelectRoom(GameProto::ClientMsg, int);
    void Work();

private:
    const std::string TABLENAME = "USER";
    const std::string SERVER_IP = "117.78.9.170";
    std::string GetServer();
    std::map<int, RecvDataManager> c2SDataMap;
};

