#include "hall_server.h"

hallServer::hallServer(Config config) : serverBase(config.hallPort) {
    for(int i = 0; i < config.serviceConfigs.size(); ++i) {
        serviceConfig tmpServiceConfig = config.serviceConfigs[i];
        serviceList.insert(service_mgr(tmpServiceConfig.ip, tmpServiceConfig.port, i+1));
    }
    oneRoomMaxUsers = config.roomconfig.roomMaxUser;
    roomNumber = config.roomconfig.roomNumber;
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
            TRACE_DETAIL(LogFormat, __FILE__, __FUNCTION__, __LINE__, "包不完整");
            break;
        }else if(headerType == -2) {
            TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "错误包 强制关闭连接");
            HandleClose(clientFd);
            close(clientFd);
            break;
        }
        int packageLength = recvDataManager.GetPackageLength();
        if(packageLength == -1) {
            TRACE_DETAIL(LogFormat, __FILE__, __FUNCTION__, __LINE__, "包不完整");
            break;
        }else if(packageLength < 0 || packageLength > BUFF_SIZE) {
            TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "错误包 强制关闭连接");
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
    for(auto service : serviceList) {
        if(service.serviceIp == std::string(addr) && service.servicePort == port) {
            ipPortAddr = std::string(addr) + ":" + std::to_string(port);
            addrFdMap[ipPortAddr] = fd;
            fdAddMap[fd] = ipPortAddr;
            break;
        }
    }
}

void hallServer::HandleClientEvent(int packageLength,int clientFd) {
    GameProto::ClientMsg clientMsg;
    int ret = clientMsg.ParseFromArray(bData.GetDataArray(), packageLength);
    if(ret < 0) {
        TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "解析协议包出错，强制关闭连接");
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
            HandleStartGame(clientMsg, clientFd);
            break;
    }
}

void hallServer::HandleServerEvent(int packageLength, int serverFd) {
    GameProto::S2SMsg s2SMsg;
    int ret = s2SMsg.ParseFromArray(bData.GetDataArray(), packageLength);
    if(ret < 0) {
        TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "解析协议包出错，强制关闭连接");
        HandleClose(serverFd);
        close(serverFd);
        return ;
    }
    switch (s2SMsg.type()) {
        case GameProto::PrepareRoom :
            break;
        case GameProto::ResultSync :
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
    printf("%s\n", sqlQuery.c_str());
    switch(ret) {
        case CONNECT_TO_SQL_ERROR:
            serverMsg.set_type(GameProto::LoginFailed_InternalError);
            serverMsg.set_str("服务器出错了");
            TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "连接数据库出错");
             //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_type(GameProto::LoginFailed_InternalError);
            TRACE_WARN(LogFormat, __FILE__, __FUNCTION__, __LINE__, "数据库语句出错");
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
            inHallUsers.insert(playerData.userid());// hall uid
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
            printf("======>_<=========  uid = %d", serverMsg.playerinfo().userid());
            //登录成功
            serverMsg.set_str("登录成功");
            break;
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
    BroadRoomListInfo();
}

void hallServer::HandleRegist(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string playerData = clientMsg.playerinfo().SerializeAsString();
    std::string sqlQuery = "insert into " + tableName + " values('" + clientMsg.playerinfo().account() + "', '" + playerData + "');";
    printf("%s\n ==========================\n", sqlQuery.c_str());
    int ret = query_sql(sqlQuery.c_str(), playerData);   
    switch(ret) {
        case CONNECT_TO_SQL_ERROR :
            serverMsg.set_type(GameProto::RegisterFailed_InternalError);
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
            //注册成功
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
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
    tempRoomInfo.roundTime = clientMsg.roominfo().roundtime();
    tempRoomInfo.roundNumber = clientMsg.roominfo().roundnumber();
    tempRoomInfo.roomName = clientMsg.roominfo().roomname();
    tempRoomInfo.mapMame = clientMsg.roominfo().mapname();
    tempRoomInfo.password = clientMsg.roominfo().password();
    tempRoomInfo.mapIdx = clientMsg.roominfo().mapidx();
    tempRoomInfo.maxPlayers = clientMsg.roominfo().maxplayers();
    tempRoomInfo.master = tempPlayerInfo.userid();
    tempRoomInfo.Players.insert(fdUserMap[clientFd]); 
    roomMap[tempRoomInfo.rid] = tempRoomInfo;
    GameProto::RoomInfo *roominfo = serverMsg.add_roominfos();
    roominfo -> CopyFrom(clientMsg.roominfo());
    roominfo -> add_players() -> CopyFrom(tempPlayerInfo);
    roominfo -> set_roomid(tempRoomInfo.rid);
    serverMsg.set_type(GameProto::CreateRoomSucceed);
    HandleSendDataToClient(serverMsg, clientFd);
    BroadRoomListInfo();
}
void hallServer::HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    int roomId = clientMsg.roominfo().roomid(), uid;
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        serverMsg.set_type(GameProto::EnterRoomFailed_DontLogin);
    }else {
        uid = fdUserMap[clientFd];
        if(roomMap.find(roomId) == roomMap.end()) {
            serverMsg.set_type(GameProto::EnterRoomFailed_RoomDontExist);
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
                roominfos->set_roomid(tempRoomInfo.rid);
                roominfos->set_roundtime(tempRoomInfo.roundTime);
                roominfos->set_roundnumber(tempRoomInfo.roundNumber);
                roominfos->set_roomname(tempRoomInfo.roomName);
                roominfos->set_password(tempRoomInfo.password);
                roominfos->set_mapname(tempRoomInfo.mapMame);
                roominfos->set_mapidx(tempRoomInfo.mapIdx);
                roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
                roominfos->set_masteruid(tempRoomInfo.master);
                roominfos->set_curplayernumber(tempRoomInfo.Players.size());
                for(auto uid : tempRoomInfo.Players) {
                // roominfos->add_preparedplayers(userInfoMap[uid]);
                    roominfos -> add_players() -> CopyFrom(userInfoMap[uid]);
                }
            }
        }
    }
    HandleSendDataToClient(serverMsg, clientFd);
    if(serverMsg.type() == GameProto::EnterRoomSucceed) {
        BroadRoomInfo(roomId);
    }
    BroadRoomListInfo();
}
void hallServer::HandleExitRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        //to do DontLogIn
    }else {
        int uid = fdUserMap[clientFd], roomId;
        if(userRoomMap.find(uid) == userRoomMap.end()) {
            //to do dontInRoom
        }else roomId = userRoomMap[uid];
        if(roomMap.find(roomId) == roomMap.end()) {
            //to do roomDontExist
        }else {
            roomInfo& tempRoomInfo = roomMap[roomId];
            if(tempRoomInfo.Players.size() == 0) {
                //roomEmpty
            }else {
                tempRoomInfo.Players.erase(uid);
                if(tempRoomInfo.Players.size() == 0) {
                    roomMap.erase(roomId);
                }else {
                   // userInfoMap[*tempRoomInfo.Players.begin()].set_prepared(true);
                    tempRoomInfo.master = *tempRoomInfo.Players.begin();
                }
                if(roomMap.find(roomId) != roomMap.end()) {
                    BroadRoomInfo(roomId);
                }
            }
        }
    }
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
void hallServer::HandleStartGame(GameProto::ClientMsg clientMsg, int clientFd) {
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        printf("================================ \n");
        //to do DontLogIn
    }else if(roomMap.find(userRoomMap[fdUserMap[clientFd]]) == roomMap.end()) {
        //to do RoomDontExist
    }else {
        int roomId = userRoomMap[fdUserMap[clientFd]];
        GameProto::ServerMsg serverMsg;
        if(roomMap[roomId].Players.size() != roomMap[roomId].maxPlayers) {
            serverMsg.set_type(GameProto::CannotStartGame);
            HandleSendDataToClient(serverMsg, clientFd);
            return ;
        }
        for(auto uid : roomMap[roomId].Players) {
            if(!userInfoMap[uid].prepared()) {
                printf("==================   not prepared\n");
                serverMsg.set_type(GameProto::CannotStartGame);
                HandleSendDataToClient(serverMsg, clientFd);
                return ;
            }
        }
        serverMsg.set_type(GameProto::CanStartGame);
        service_mgr tempService = *serviceList.begin();
        serviceList.erase(tempService);
        tempService.servicePressure += roomMap[roomId].Players.size();
        GameProto::S2SMsg sendS2SMsg;
        sendS2SMsg.set_type(GameProto::PrepareRoom);
        for(auto uid : roomMap[roomId].Players) {
            serverMsg.clear_playerinfo();
            //serverMsg.CopyFrom(userInfoMap[uid]);
            serverMsg.set_str(tempService.serviceIp + ":" + std::to_string(tempService.servicePort)); 
            serverMsg.set_ip(tempService.serviceIp);
            serverMsg.set_port(tempService.servicePort);
            GameProto::RoomInfo roominfo;
            roominfo.set_roomid(roomId);
            roominfo.set_maxplayers(roomMap[roomId].maxPlayers);
            serverMsg.add_roominfos()->CopyFrom(roominfo);
            printf("!==================================! %d %d %d\n", serverMsg.roominfos(0).maxplayers(), roominfo.maxplayers(), roomMap[roomId].maxPlayers);
            HandleSendDataToClient(serverMsg, userFdMap[uid]);
            onlineUsers.erase(uid); 
            fdUserMap.erase(uid);
        }
        roomMap.erase(roomId);
        serviceList.insert(tempService);
    }
}

void hallServer::BroadRoomInfo(int roomId) {
    GameProto::ServerMsg serverMsg;
    roomInfo &tempRoomInfo = roomMap[roomId];
    serverMsg.set_type(GameProto::BroadRoomInfo);
    GameProto::RoomInfo* roominfos = serverMsg.add_roominfos();
    roominfos->set_roomid(tempRoomInfo.rid);
    roominfos->set_roundtime(tempRoomInfo.roundTime);
    roominfos->set_roundnumber(tempRoomInfo.roundNumber);
    roominfos->set_roomname(tempRoomInfo.roomName);
    roominfos->set_mapidx(tempRoomInfo.mapIdx);
    roominfos->set_password(tempRoomInfo.password);
    roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
    roominfos->set_masteruid(tempRoomInfo.master);
    roominfos->set_curplayernumber(tempRoomInfo.Players.size());
    for(auto uid : tempRoomInfo.Players) {
        // roominfos->add_preparedplayers(userInfoMap[uid]);
        roominfos->add_players()->CopyFrom(userInfoMap[uid]);
    }
    for(auto uid : tempRoomInfo.Players) {
        HandleSendDataToClient(serverMsg, userFdMap[uid]);
    }
}
void hallServer::BroadRoomListInfo() {
    GameProto::ServerMsg serverMsg;
    serverMsg.set_type(GameProto::BroadRoomList);
    serverMsg.set_str("广播房间信息");
    GameProto::RoomInfo* roominfos;
    for(auto val : roomMap) {
        roominfos = serverMsg.add_roominfos();
        roomInfo tempRoomInfo = val.second;
        roominfos->set_roomid(tempRoomInfo.rid);
        roominfos->set_roundtime(tempRoomInfo.roundTime);
        roominfos->set_roundnumber(tempRoomInfo.roundNumber);
        roominfos->set_roomname(tempRoomInfo.roomName);
        roominfos->set_password(tempRoomInfo.password);
        roominfos->set_mapname(tempRoomInfo.mapMame);
        roominfos->set_mapidx(tempRoomInfo.mapIdx);
        roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
        roominfos->set_masteruid(tempRoomInfo.master);
        roominfos->set_curplayernumber(tempRoomInfo.Players.size());
    }
    for(auto uid : onlineUsers) {
        HandleSendDataToClient(serverMsg, userFdMap[uid]);
    }
}

void hallServer::HandleSendDataToClient(GameProto::ServerMsg serverMsg, int clientFd) {
    for(int i = 1; i < HEAD_LENGTH; ++i) {
        bData.ChangeBuffAt(serverMsg.ByteSize() >> ((i -1) * 8), i);
    }
    int ret = serverMsg.SerializeToArray(bData.GetBuffArray() + HEAD_LENGTH, BUFF_SIZE);
    SendDataToClient(clientFd, HEAD_LENGTH + serverMsg.ByteSize());
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

void hallServer::HandleClose(int clientFd) {
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
        inHallUsers.erase(clientUid);
        userInfoMap.erase(clientUid);
        fdUserMap.erase(clientFd);
    }
}

void hallServer::Work() {
    serverBase::Work();
}

