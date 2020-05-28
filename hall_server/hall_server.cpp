#include "hall_server.h"

hallServer::hallServer(Config config) : serverBase(config.hallPort) {
    srand((unsigned)time(NULL));
    for(int i = 0; i < config.serviceConfigs.size(); ++i) {
        serviceConfig tmpServiceConfig = config.serviceConfigs[i];
        serviceList.insert(service_mgr(tmpServiceConfig.ip, tmpServiceConfig.port, i+1));
    }
    tableName = config.tableName;
    servicePressureLimit = config.servicePressureLimit;
}

void hallServer::HandleEvent(int clientFd, int dataLength) {
    if(c2SDataMap.find(clientFd) == c2SDataMap.end()) {
        c2SDataMap[clientFd] = recvDataManager();    
    }
    recvDataManager &recvDataManager = c2SDataMap[clientFd];
    for(int i = 0; i < dataLength; ++i) {
        recvDataManager.PushByte(bData.GetBuffCharAt(i));
    }
    while(true) {
        int headerType = recvDataManager.GetHeaderType();
        if(headerType == -1) {
            TRACE_WARN(LOGFORMAT, "包不完整");
            break;
        }else if(headerType == -2) {
            TRACE_WARN(LOGFORMAT, "错误包， 强制关闭连接");
            HandleClose(clientFd);
            close(clientFd);
            break;
        }
        int packageLength = recvDataManager.GetPackageLength();
        if(packageLength == -1) {
            TRACE_WARN(LOGFORMAT, "包不完整");
            break;
        }else if(packageLength < 0 || packageLength > BUFF_SIZE) {
            TRACE_WARN(LOGFORMAT, "错误包， 强制关闭连接");
            HandleClose(clientFd);
            close(clientFd);
            break;
        }
        for(int i = 0; i < HEAD_LENGTH; ++i) {
            recvDataManager.PopByte();
        }
        for(int i = 0; i < packageLength; ++i) {
            bData.ChangeDataAt(recvDataManager.PopByte(), i);
        }
        sprintf(logCache, "headerType is %d", headerType);
        TRACE_DETAIL(LOGFORMAT, logCache);
        if(headerType == C2SType) {
            HandleClientEvent(packageLength, clientFd);
        }else {
            HandleServerEvent(packageLength, clientFd);
        }
        break;
    }
}

void hallServer::HandleNetIp(char* addr, int port, int fd) {
    std::string ipPortAddr = "";
    sprintf(logCache, "new connect come addr = %s, port = %d, fd = %d", addr, port, fd);
    TRACE_DETAIL(LOGFORMAT, logCache);
    for(auto service : serviceList) {
        printf("%s %s \n", service.serviceIp.c_str(), addr);
        if(service.serviceIp == std::string(addr)) {

            TRACE_DETAIL(LOGFORMAT, logCache);
            sFd.insert(fd);
            break;
        }
    }
}

void hallServer::HandleClientEvent(int packageLength,int clientFd) {
    GameProto::ClientMsg clientMsg;
    int ret = clientMsg.ParseFromArray(bData.GetDataArray(), packageLength);
    if(ret < 0) {
        TRACE_WARN(LOGFORMAT, "C2S解析协议包出错,强制关闭连接");
        //to do ParseFromArray Error
        HandleClose(clientFd); 
        close(clientFd);
        return ;
    }
    switch(clientMsg.type()) {
        case GameProto::Login :
            HandleLogIn(clientMsg, clientFd);
            break;
        case GameProto::Regist :
            HandleRegist(clientMsg, clientFd);
            break;
        case GameProto::CreateRoom :
            HandleCreateRoom(clientMsg, clientFd);
            break;
        case GameProto::EnterRoom :
            HandleSelectRoom(clientMsg, clientFd);
            break;
        case GameProto::ExitRoom :
            HandleExitRoom(clientMsg, clientFd);
            break;
        case GameProto::ChangeStateInRoom:
            HandleChangeStateInRoom(clientMsg, clientFd);
            break;
        case GameProto::StartGameBtnOnClick:
            HandlePrepareStartGame(clientMsg, clientFd);
            break;
        case GameProto::C2SSync:
            HandleReLogIn(clientMsg, clientFd);
    }
}

void hallServer::HandleServerEvent(int packageLength, int serverFd) {
    GameProto::S2SMsg s2SMsg;
    int ret = s2SMsg.ParseFromArray(bData.GetDataArray(), packageLength);
    if(ret < 0) {
        TRACE_WARN(LOGFORMAT, "S2S解析协议包出错,强制关闭连接");
        HandleClose(serverFd);
        close(serverFd);
        return ;
    }
    switch (s2SMsg.type()) {
        case GameProto::PrepareRoom:
            HandleTrueStartGame(s2SMsg);
            break;
        case GameProto::ResultSync:
            HandleResultSync(s2SMsg);
            break;
    }

}


void hallServer::HandleLogIn(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string queryResult;
    std::string sqlQuery = "select * from " + tableName + " where account = '" + clientMsg.playerinfo().account() + "';";
    GameProto::PlayerInfo playerData;
    int ret = query_sql(sqlQuery.c_str(), queryResult);
    playerData.ParseFromString(queryResult);
    TRACE_DETAIL(LOGFORMAT, queryResult.c_str());
    switch(ret) {
        case CONNECT_TO_SQL_ERROR:
            serverMsg.set_type(GameProto::LoginFailed_InternalError);
            serverMsg.set_str("服务器出错了");
            TRACE_WARN(LOGFORMAT, "连接数据库出错");
             //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_type(GameProto::LoginFailed_InternalError);
            TRACE_WARN(LOGFORMAT, "数据库语句出错");
            serverMsg.set_str("服务器出错了");
            //数据库语句出错
            break;
        case QUERY_EMPTY:
            serverMsg.set_type(GameProto::LoginFailed_AccountDontExist);
            serverMsg.set_str("账号不存在");
            //用户账号或密码错误
            break;
        case QUERY_OK:
            if(clientMsg.playerinfo().password() != playerData.password()) {
                serverMsg.set_type(GameProto::LoginFailed_PasswordWrong);
                serverMsg.set_str("密码错误");
                break;
            }
            if(loginAccount.find(clientMsg.playerinfo().account()) != loginAccount.end()) {
                serverMsg.set_type(GameProto::LoginFailed_AlreadyLogin);
                serverMsg.set_str("用户已登录");
                break;
            }
            

            playerData.set_userid(GetUid());
            playerData.clear_password();
            fdUserMap[clientFd] = playerData.userid(); //fd -> uid
            userFdMap[playerData.userid()] = clientFd; //uid -> fd
            uidAccountMap[playerData.userid()] = playerData.account(); //uid -> account
            userInfoMap[playerData.userid()] = playerData; //uid -> playerinfo
            onlineUsers.insert(playerData.userid()); //online uid
           // inHallUsers.insert(playerData.userid());// hall uid
            loginAccount.insert(playerData.account()); //loginAccount
            serverMsg.set_type(GameProto::LoginSucceed);
            GameProto::PlayerInfo *ptr = serverMsg.mutable_playerinfo();
            ptr -> CopyFrom(playerData);
            /*
            if(accountRoomMap.find(playerData.account()) != accountRoomMap.end()) {
                serverMsg.set_str(roomMap[accountRoomMap[playerData.account()]].serviceAddr);
                userRoomMap[playerData.uid()] = accountRoomMap[playerData.account()]; 
                inHallUsers.erase(playerData.uid());
            } else {
            }
            */
            if(startedUserRoomMap.find(playerData.userid()) != startedUserRoomMap.end()) {
                int roomId = startedUserRoomMap[playerData.userid()];
                if(startedRoomMap.find(roomId) != startedRoomMap.end()) {
                    startedRoomMap[roomId].Players.erase(playerData.userid());
                    if(startedRoomMap[roomId].Players.size() == 0) {
                        startedRoomMap.erase(roomId);
                    }
                }
                startedUserRoomMap.erase(playerData.userid());
            }
            sprintf(logCache, "new login, the account is %s, the uid is %d", clientMsg.playerinfo().account().c_str(), serverMsg.playerinfo().userid());
            TRACE_DETAIL(LOGFORMAT, logCache);
            //登录成功
            serverMsg.set_str("登录成功");
            break;
    }
    sprintf(logCache, "返回登录信息 %s", serverMsg.str().c_str());
    TRACE_DETAIL(LOGFORMAT, logCache);
    HandleSendData(0, serverMsg, clientFd);
    BroadRoomListInfo();
}

void hallServer::HandleRegist(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string playerData = clientMsg.playerinfo().SerializeAsString();
    std::string sqlQuery = "insert into " + tableName + " values('" + clientMsg.playerinfo().account() + "', '" + playerData + "');";
    TRACE_DETAIL(LOGFORMAT, sqlQuery.c_str());
    int ret = query_sql(sqlQuery.c_str(), playerData);   
    switch(ret) {
        case CONNECT_TO_SQL_ERROR :
            serverMsg.set_type(GameProto::RegisterFailed_InternalError);
            TRACE_WARN(LOGFORMAT, "连接数据库出错");
            serverMsg.set_str("服务器出错了");
            //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_type(GameProto::RegisterFailed_AccountAlreadyExist);
            serverMsg.set_str("用户已存在");
            //用户已存在
            break;
        default:
            serverMsg.set_type(GameProto::RegisterSucceed);
            serverMsg.set_str("注册成功");
            sprintf(logCache, "new register, account is %s", clientMsg.playerinfo().account().c_str());
            TRACE_DETAIL(LOGFORMAT, logCache);
            //注册成功
    }
    sprintf(logCache, "返回注册信息 %s", serverMsg.str().c_str());
    printf("%s\n", serverMsg.str().c_str());
    HandleSendData(0, serverMsg, clientFd);
}

void hallServer::HandleCreateRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    roomInfo tempRoomInfo;
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        //to do DontLogIn
        return ;
    }
    for(auto rid : roomMap) {
        if(rid.second.Players.find(fdUserMap[clientFd]) != rid.second.Players.end()) {
            return ;
        }
    }

    GameProto::PlayerInfo &tempPlayerInfo = userInfoMap[fdUserMap[clientFd]];
    userRoomMap[tempPlayerInfo.userid()] = tempRoomInfo.rid;

    tempPlayerInfo.set_roomid(tempRoomInfo.rid);
    tempRoomInfo.SetFromProto(clientMsg.roominfo());
    /*
    tempRoomInfo.roundTime = clientMsg.roominfo().roundtime();
    tempRoomInfo.roundNumber = clientMsg.roominfo().roundnumber();
    tempRoomInfo.roomName = clientMsg.roominfo().roomname();
    tempRoomInfo.mapName = clientMsg.roominfo().mapname();
    tempRoomInfo.password = clientMsg.roominfo().password();
    tempRoomInfo.mapIdx = clientMsg.roominfo().mapidx();
    tempRoomInfo.maxPlayers = clientMsg.roominfo().maxplayers();
    */
    tempRoomInfo.master = tempPlayerInfo.userid();
    tempRoomInfo.Players.insert(fdUserMap[clientFd]); 
    roomMap[tempRoomInfo.rid] = tempRoomInfo;
    GameProto::RoomInfo *roominfo = serverMsg.add_roominfos();
    roominfo -> CopyFrom(clientMsg.roominfo());
    roominfo -> add_players() -> CopyFrom(tempPlayerInfo);
    roominfo -> set_roomid(tempRoomInfo.rid);
    serverMsg.set_type(GameProto::CreateRoomSucceed);
    sprintf(logCache, "创建房间成功 房间id= %d 房间名= %s", tempRoomInfo.rid, tempRoomInfo.mapName.c_str());
    TRACE_DETAIL(LOGFORMAT, logCache);
    printf("======== %d\n", roomMap[tempRoomInfo.rid].rid);
    HandleSendData(0, serverMsg, clientFd);
    BroadRoomListInfo();
}
void hallServer::HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    printf("=======------=====\n");
    int roomId = clientMsg.roominfo().roomid(), uid;
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        sprintf(logCache, "dont login");
        serverMsg.set_type(GameProto::EnterRoomFailed_DontLogin);
    }else {
        uid = fdUserMap[clientFd];
        if(roomMap.find(roomId) == roomMap.end()) {
            serverMsg.set_type(GameProto::EnterRoomFailed_RoomDontExist);
            sprintf(logCache, "room dont exist roomid = %d", roomId);
            //to do roomDontExist
        }else {
            roomInfo& tempRoomInfo = roomMap[roomId];
            if(tempRoomInfo.Players.size() == tempRoomInfo.maxPlayers) {
                serverMsg.set_type(GameProto::EnterRoomFailed_RoomIsFull);
              //to do roomFull
            }else {
                for(auto room : roomMap) {
                    if(room.second.Players.find(uid) != room.second.Players.end()) {
                        //to do InRoom;
                        return ;
                    }
                }
                tempRoomInfo.Players.insert(uid);
                //to do EnterRoom
                serverMsg.set_type(GameProto::EnterRoomSucceed);
                roomInfo &tempRoomInfo = roomMap[roomId];
                userRoomMap[uid] = roomId;
      //  serverMsg.set_type(GameProto::BroadRoomListInfo);
                GameProto::RoomInfo* roominfos = serverMsg.add_roominfos();

                tempRoomInfo.GetProto(*roominfos);
                /*
                roominfos->set_roomid(tempRoomInfo.rid);
                roominfos->set_roundtime(tempRoomInfo.roundTime);
                roominfos->set_roundnumber(tempRoomInfo.roundNumber);
                roominfos->set_roomname(tempRoomInfo.roomName);
                roominfos->set_password(tempRoomInfo.password);
                roominfos->set_mapname(tempRoomInfo.mapName);
                roominfos->set_mapidx(tempRoomInfo.mapIdx);
                roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
                roominfos->set_masteruid(tempRoomInfo.master);
                roominfos->set_curplayernumber(tempRoomInfo.Players.size());
                */
                for(auto uid : tempRoomInfo.Players) {
                // roominfos->add_preparedplayers(userInfoMap[uid]);
                    roominfos -> add_players() -> CopyFrom(userInfoMap[uid]);
                }
            }
        }
    }
    HandleSendData(0, serverMsg, clientFd);
    if(serverMsg.type() == GameProto::EnterRoomSucceed) {
        BroadRoomInfo(roomId);
    }
    BroadRoomListInfo();
}
void hallServer::HandleExitRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    printf("========= exitRoom\n");
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        sprintf(logCache, "Dont Log In fd = %s", clientFd);
        //to do DontLogIn
    }else {
        int uid = fdUserMap[clientFd], roomId;
        if(userRoomMap.find(uid) == userRoomMap.end()) {
            sprintf(logCache, "dot in room uid = %d", uid);
            //to do dontInRoom
        }else roomId = userRoomMap[uid];
        if(roomMap.find(roomId) == roomMap.end()) {
            sprintf(logCache, "room dont exist roomid = %d", roomId);
            //to do roomDontExist
        }else {
            sprintf(logCache, "ok");
            roomInfo& tempRoomInfo = roomMap[roomId];
            if(tempRoomInfo.Players.size() == 0) {
                roomMap.erase(roomId);
                //roomEmpty
            }else {
                tempRoomInfo.Players.erase(uid);
                if(tempRoomInfo.Players.size() == 0) {
                    roomMap.erase(roomId);
                }else {
                   // userInfoMap[*tempRoomInfo.Players.begin()].set_prepared(true);
                    tempRoomInfo.master = *tempRoomInfo.Players.begin();
                    BroadRoomInfo(roomId);
                }
            }
        }
    }
    TRACE_DETAIL(LOGFORMAT, logCache);
    //HandleSendDataToClient(serverMsg, clientFd);
    //if(serverMsg.type() == GameProto::ExitRoomSuccess) {
    //    BroadRoomInfo(roomId);
    //}
    BroadRoomListInfo();
}
void hallServer::HandleChangeStateInRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        //to do DontLogIn
    }else {
        int uid = fdUserMap[clientFd];
        if(userRoomMap.find(uid) == userRoomMap.end()) {
            //to do DontInRoom
        }else {
            int roomId = userRoomMap[uid];
            GameProto::PlayerInfo &tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfo.CopyFrom(clientMsg.playerinfo());
            printf("================= ChangeState %d %d\n", tempPlayerInfo.characterid(), tempPlayerInfo.prepared());
            tempPlayerInfo.set_userid(uid);
            tempPlayerInfo.set_account(uidAccountMap[uid]);
            BroadRoomInfo(roomId);
        }
    }
}
void hallServer::HandlePrepareStartGame(GameProto::ClientMsg clientMsg, int clientFd) {
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        sprintf(logCache, "clientFd : %d has not login", clientFd);
        TRACE_WARN(LOGFORMAT, logCache);
        //to do DontLogIn
    }else if(roomMap.find(userRoomMap[fdUserMap[clientFd]]) == roomMap.end()) {
        sprintf(logCache, "房间不存在 房间id = %d", userRoomMap[fdUserMap[clientFd]]);
        TRACE_WARN(LOGFORMAT, logCache);
        //to do RoomDontExist
    
    }else if(onPrepareRoom.find(userRoomMap[fdUserMap[clientFd]]) != onPrepareRoom.end()) {
        sprintf(logCache, "游戏一开始 房间id= %d", userRoomMap[fdUserMap[clientFd]]);
        TRACE_WARN(LOGFORMAT, logCache);
    }else {
        int roomId = userRoomMap[fdUserMap[clientFd]];
        GameProto::ServerMsg serverMsg;
        for(auto uid : roomMap[roomId].Players) {
            if(!userInfoMap[uid].prepared()) {
                TRACE_DETAIL(LOGFORMAT, "还有玩家未准备");
                serverMsg.set_type(GameProto::CannotStartGame);
                HandleSendData(0, serverMsg, clientFd);
                return ;
            }
        }
       // serverMsg.set_type(GameProto::CanStartGame);
        service_mgr tempService = *serviceList.begin();
        serviceList.erase(tempService);
        tempService.servicePressure += roomMap[roomId].Players.size();
        serviceList.insert(tempService);
        roomMap[roomId].serviceAddr = tempService.serviceIp + ":" + std::to_string(tempService.servicePort);
        roomMap[roomId].ip = tempService.serviceIp;
        roomMap[roomId].port = tempService.servicePort;

        GameProto::S2SMsg sendS2SMsg;
        sendS2SMsg.set_type(GameProto::PrepareRoom);
        GameProto::RoomInfo *roomPtr =  sendS2SMsg.mutable_roominfo();
        GameProto::RoomInfo tempRoominfo;
        roomMap[roomId].GetProto(tempRoominfo);
        for(auto uid : roomMap[roomId].Players) {
            tempRoominfo.add_players() -> CopyFrom(userInfoMap[uid]);
        }
        roomPtr -> CopyFrom(tempRoominfo);
        sprintf(logCache, "开始游戏 房间id = %d 人数 = %d roomid = %d", roomId, sendS2SMsg.roominfo().curplayernumber(), sendS2SMsg.roominfo().roomid());
        TRACE_DETAIL(LOGFORMAT, logCache);
        if(sFd.size() == 0) {
           TRACE_WARN(LOGFORMAT, "战斗服务器还未准备好");
        }else if(onPrepareRoom.find(roomId) == onPrepareRoom.end()) {
            HandleSendData(1, sendS2SMsg, *sFd.begin());
            onPrepareRoom.insert(roomId);
        }


 //       GameProto::S2SMsg s2SMsg;
  //      s2SMsg.mutable_roominfo()->set_roomid(roomId);
   //     HandleTrueStartGame(s2SMsg);
    }
}

void hallServer::HandleTrueStartGame(GameProto::S2SMsg s2SMsg) {    
    sprintf(logCache, "HandleTrueStartGame roomid = %d", s2SMsg.roominfo().roomid());
    TRACE_DETAIL(LOGFORMAT, logCache);
    int roomId = s2SMsg.roominfo().roomid();
    if(roomMap.find(roomId) == roomMap.end()) {
        sprintf(logCache, "room dont exist, roomid = %d", roomId);
        TRACE_WARN(LOGFORMAT, logCache);
    }else {
        GameProto::ServerMsg serverMsg;
        serverMsg.set_type(GameProto::CanStartGame);
        serverMsg.set_str(roomMap[roomId].serviceAddr);
        serverMsg.set_ip(roomMap[roomId].ip);
        serverMsg.set_port(roomMap[roomId].port);
        serverMsg.add_roominfos() -> set_roomid(roomId);
        serverMsg.set_nonce(random());
        serverMsg.set_nonce(rand());
        for(auto uid : roomMap[roomId].Players) {
            startedUserRoomMap[uid] = roomId;
            HandleSendData(0, serverMsg, userFdMap[uid]); 
        }
    //    onGameRoom[roomId] = roomMap[roomId];
        startedRoomMap[roomId] = roomMap[roomId];
        roomMap.erase(roomId);
        onPrepareRoom.erase(roomId);
    }
}

void hallServer::HandleReLogIn(GameProto::ClientMsg clientMsg, int clientFd) {
    std::string queryResult;
    std::string sqlQuery = "select * from " + tableName + " where account = '" + clientMsg.playerinfo().account() + "';";
    GameProto::PlayerInfo playerData;
    int ret = query_sql(sqlQuery.c_str(), queryResult);
    playerData.ParseFromString(queryResult);
    sprintf(logCache, "new RelogIn Come account = %s, NickName = %s", clientMsg.playerinfo().account().c_str(), clientMsg.playerinfo().nickname().c_str());
    TRACE_DETAIL(LOGFORMAT, logCache);
    switch(ret) {
        case QUERY_OK:
            int uid = clientMsg.playerinfo().userid();
            if(startedUserRoomMap.find(uid) != startedUserRoomMap.end()) {
                int roomId = startedUserRoomMap[uid];
                if(startedRoomMap.find(roomId) != startedRoomMap.end()) {
                    if(roomMap.find(roomId) == roomMap.end()) {
                        roomMap[roomId] = startedRoomMap[roomId];
                        roomMap[roomId].Players.clear();
                        roomMap[roomId].master = uid;
                    }
                    startedRoomMap[roomId].Players.erase(uid);
                    startedUserRoomMap.erase(uid);
                    userFdMap[uid] = clientFd;
                    fdUserMap[clientFd] = uid;
                    roomMap[roomId].Players.insert(uid);
                    userRoomMap[uid] = roomId;
                    uidAccountMap[uid] = clientMsg.playerinfo().account();
                    GameProto::PlayerInfo playerData;
                    playerData.ParseFromString(queryResult);
                    playerData.clear_password();
                    userInfoMap[uid] = playerData;
                    onlineUsers.insert(uid);
                    BroadRoomInfo(roomId);
                    BroadRoomListInfo();
            }
        }
        break;
    } 
}
void hallServer::HandleResultSync(GameProto::S2SMsg s2SMsg) {
    GameProto::S2SMsg sendS2SMsg;
    sendS2SMsg.set_type(GameProto::ResultSync);
    for(int i = 0; i < s2SMsg.roominfo().curplayernumber(); ++i) {
        GameProto::PlayerInfo playerino = s2SMsg.roominfo().players(i);
    }
    HandleSendData(1, sendS2SMsg, *sFd.begin());
}

void hallServer::BroadRoomInfo(int roomId) {
    GameProto::ServerMsg serverMsg;
    roomInfo &tempRoomInfo = roomMap[roomId];
    serverMsg.set_type(GameProto::BroadRoomInfo);
    GameProto::RoomInfo* roominfoPtr = serverMsg.add_roominfos();
    GameProto::RoomInfo roominfo;
    tempRoomInfo.GetProto(*roominfoPtr);
    for(auto uid : tempRoomInfo.Players) {
        // roominfos->add_preparedplayers(userInfoMap[uid]);
        roominfoPtr->add_players()->CopyFrom(userInfoMap[uid]);
    }

    for(auto uid : tempRoomInfo.Players) {
        HandleSendData(0, serverMsg, userFdMap[uid]);
    }
}
void hallServer::BroadRoomListInfo() {
    GameProto::ServerMsg serverMsg;
    serverMsg.set_type(GameProto::BroadRoomList);
    serverMsg.set_str("广播房间信息");
    GameProto::RoomInfo* roominfos;
    for(auto val : roomMap) {
        roominfos = serverMsg.add_roominfos();
        roomInfo &tempRoomInfo = val.second;
        tempRoomInfo.GetProto(*roominfos);
        /*
        roominfos->set_roomid(tempRoomInfo.rid);
        roominfos->set_roundtime(tempRoomInfo.roundTime);
        roominfos->set_roundnumber(tempRoomInfo.roundNumber);
        roominfos->set_roomname(tempRoomInfo.roomName);
        roominfos->set_password(tempRoomInfo.password);
        roominfos->set_mapname(tempRoomInfo.mapName);
        roominfos->set_mapidx(tempRoomInfo.mapIdx);
        roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
        roominfos->set_masteruid(tempRoomInfo.master);
        roominfos->set_curplayernumber(tempRoomInfo.Players.size());
        */
    }
    for(auto uid : onlineUsers) {
        HandleSendData(0, serverMsg, userFdMap[uid]);
    }
}

void hallServer::HandleSelectCharacter(GameProto::ClientMsg clientMsg, int clientFd) {
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        //to do DontLogIn
    }else {
        int uid = fdUserMap[uid];
        if(userRoomMap.find(uid) == userRoomMap.end()) {
            //to do DontInRoom
        }else {
            int roomId = userRoomMap[uid];
            GameProto::PlayerInfo &tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfo.set_charactername(clientMsg.playerinfo().charactername());
            tempPlayerInfo.set_characterid(clientMsg.playerinfo().characterid());
            BroadRoomInfo(roomId);
        }
    }
}

void hallServer::HandleSendData(int symbol, google::protobuf::Message &msg, int fd) {
    printf("=================== %d %d  %d\n", symbol, msg.ByteSize(),fd);
    bData.ChangeBuffAt(symbol, 0);
    for(int i = 1; i < HEAD_LENGTH; ++i) {
        bData.ChangeBuffAt(msg.ByteSize() >> ((i -1) * 8), i);
    }
    int ret = msg.SerializeToArray(bData.GetBuffArray() + HEAD_LENGTH, BUFF_SIZE);
    if(ret < 0) {
        sprintf(logCache, "发送数据给fd: %d, 序列化失败", fd);
        TRACE_WARN(LOGFORMAT, logCache);
        return ;
    }
    SendDataToClient(fd, HEAD_LENGTH + msg.ByteSize());
}

void hallServer::HandleClose(int clientFd) {
   // sprintf(logCache, )
    TRACE_DETAIL(LOGFORMAT, logCache);
    c2SDataMap.erase(clientFd);
    if(fdUserMap.find(clientFd) != fdUserMap.end()) {
        int clientUid = fdUserMap[clientFd];
        if(userRoomMap.find(clientUid) != userRoomMap.end()) {
            int clientRid = userRoomMap[clientUid];
            roomMap[clientRid].Players.erase(clientUid);
            if(roomMap[clientRid].Players.size() == 0) {
                roomMap.erase(clientRid);
            }
            userRoomMap.erase(clientUid);
        }   
        loginAccount.erase(uidAccountMap[clientUid]);
        uidAccountMap.erase(clientUid);
        userFdMap.erase(clientUid);
        onlineUsers.erase(clientUid);
        //inHallUsers.erase(clientUid);
        userInfoMap.erase(clientUid);
        fdUserMap.erase(clientFd);
    }
    sFd.erase(clientFd);
}

void hallServer::Work() {
    serverBase::Work();
}

