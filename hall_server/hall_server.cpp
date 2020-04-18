#include "hall_server.h"

hallServer::hallServer(Config config) : serverBase(config.hallPort) {
    for(int i = 0; i < config.serviceConfigs.size(); ++i) {
        serviceConfig tmpServiceConfig = config.serviceConfigs[i];
        serviceList.push_back(service_mgr(tmpServiceConfig.ip, tmpServiceConfig.port, i+1));
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
    GameProto::ClientMsg clientMsg;
    while(true) {
        int packageLength = recvDataManager.GetPackageLength();
        if(packageLength == -1) {
            break;
        }
        printf("========================== %d\n", packageLength);
        for(int i = 0; i < HEAD_LENGTH; ++i) {
            recvDataManager.PopByte();
        }
        for(int i = 0; i < packageLength; ++i) {
            bData.ChangeDataAt(recvDataManager.PopByte(), i);
        }
        int ret = clientMsg.ParseFromArray(bData.GetDataArray(), packageLength);
        switch(clientMsg.type()) {
            case GameProto::LogIn :
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
            case GameProto::StartGame:
                HandleStartGame(clientMsg, clientFd);
                break;
        }
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
            serverMsg.set_type(GameProto::InternalError);
            serverMsg.set_str("服务器出错了");
             //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_type(GameProto::InternalError);
            serverMsg.set_str("服务器出错了");
            //数据库语句出错
            break;
        case QUERY_EMPTY:
            serverMsg.set_type(GameProto::LogInError_AccountDontExist);
            serverMsg.set_str("账号不存在");
            //用户账号或密码错误
            break;
        case QUERY_OK:
            if(clientMsg.playerinfo().password() != playerData.password()) {
                serverMsg.set_type(GameProto::LogInError_PasswordWrong);
                serverMsg.set_str("密码错误");
                break;
            }
            if(loginAccount.find(clientMsg.playerinfo().account()) != loginAccount.end()) {
                serverMsg.set_type(GameProto::LogInError_ReLogIn);
                serverMsg.set_str("用户已登录");
                break;
            }
            

            playerInfo tempPlayerInfo = playerInfo(playerData.account(), playerData.nickname());
            fdUserMap[clientFd] = tempPlayerInfo.uid; //fd -> uid
            playerData.set_uid(tempPlayerInfo.uid); //protobuf <- uid
            userFdMap[tempPlayerInfo.uid] = clientFd; //uid -> fd
            uidAccountMap[tempPlayerInfo.uid] = playerData.account(); //uid -> account
            userInfoMap[tempPlayerInfo.uid] = tempPlayerInfo; //uid -> playerinfo
            onlineUsers.insert(tempPlayerInfo.uid); //online uid
            inHallUsers.insert(tempPlayerInfo.uid);// hall uid
            loginAccount.insert(playerData.account()); //loginAccount
            serverMsg.set_type(GameProto::LogInSuccess);
            GameProto::PlayerInfo *ptr = serverMsg.mutable_playerinfo();
            ptr -> CopyFrom(playerData);
            if(accountRoomMap.find(playerData.account()) != accountRoomMap.end()) {
                serverMsg.set_str(roomMap[accountRoomMap[playerData.account()]].serviceAddr);
                userRoomMap[tempPlayerInfo.uid] = accountRoomMap[tempPlayerInfo.GetAccount()]; 
                inHallUsers.erase(tempPlayerInfo.uid);
            } else {
                serverMsg.set_str("登录成功");
            }
            //登录成功
            break;
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
    BroadRoomInfo();
}

void hallServer::HandleRegist(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string playerData = clientMsg.playerinfo().SerializeAsString();
    std::string sqlQuery = "insert into " + tableName + " values('" + clientMsg.playerinfo().account() + "', '" + playerData + "');";
    printf("%s\n ==========================\n", sqlQuery.c_str());
    int ret = query_sql(sqlQuery.c_str(), playerData);   
    switch(ret) {
        case CONNECT_TO_SQL_ERROR :
            serverMsg.set_type(GameProto::InternalError);
            serverMsg.set_str("服务器出错了");
            //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_type(GameProto::RegisterError_AccountAlreadyExist);
            serverMsg.set_str("用户已存在");
            //用户已存在
            break;
        default:
            serverMsg.set_type(GameProto::RegisterSuccess);
            serverMsg.set_str("注册成功");
            //注册成功
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
}

void hallServer::HandleCreateRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    roomInfo tempRoomInfo;
    GameProto::ServerMsg serverMsg;
    GameProto::PlayerInfo tempPlayerInfoProto;
    playerInfo &tempPlayerInfo = userInfoMap[fdUserMap[clientFd]];

///==========================================
    tempPlayerInfo.rid = tempRoomInfo.rid;
    tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
    tempPlayerInfoProto.set_roomid(tempRoomInfo.rid);
    tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
///==========================================
    tempRoomInfo.roundTime = clientMsg.roominfo().roundtime();
    tempRoomInfo.roundNumber = clientMsg.roominfo().roundnumber();
    tempRoomInfo.roomName = clientMsg.roominfo().roomname();
    tempRoomInfo.mapMame = clientMsg.roominfo().mapname();
    tempRoomInfo.password = clientMsg.roominfo().password();
    tempRoomInfo.maxPlayers = clientMsg.roominfo().maxplayers();
    tempRoomInfo.master = tempPlayerInfo.uid;
    tempRoomInfo.preparedPlayers.insert(fdUserMap[clientFd]); 
    std::cout << tempRoomInfo.roundTime << " " << tempRoomInfo.roundNumber <<" " << tempRoomInfo.roomName <<" " << tempRoomInfo.mapMame <<" " << tempRoomInfo.password <<" =  " << tempPlayerInfo.rid << std::endl;
    roomMap[tempRoomInfo.rid] = tempRoomInfo;
    GameProto::RoomInfo *roominfo = serverMsg.add_roominfos();
    roominfo -> CopyFrom(clientMsg.roominfo());
    roominfo -> add_preparedplayers() -> CopyFrom(tempPlayerInfoProto);
    roominfo -> set_id(tempRoomInfo.rid);
    serverMsg.set_type(GameProto::CreateRoomSuccess);
    std::cout << " ======================== " << std::endl;
    HandleSendDataToClient(serverMsg, clientFd);
    BroadRoomInfo();
}
void hallServer::HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    int roomId = clientMsg.roominfo().id(), uid;
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        serverMsg.set_type(GameProto::EnterRoomError_DontLogIn);
    }else {
        uid = fdUserMap[clientFd];
        if(roomMap.find(roomId) == roomMap.end()) {
            serverMsg.set_type(GameProto::EnterRoomError_RoomDontExit);
            //to do roomDontExist
        }else {
            roomInfo& tempRoomInfo = roomMap[roomId];
            printf(" ====================\n %s\n", tempRoomInfo.mapMame.c_str());
            if(tempRoomInfo.preparedPlayers.size() + tempRoomInfo.unPreparedPlayers.size() == tempRoomInfo.maxPlayers) {
                serverMsg.set_type(GameProto::EnterRoomError_RoomIsFull);
              //to do roomFull
            }else {
                tempRoomInfo.unPreparedPlayers.insert(uid);
                //to do EnterRoom
                serverMsg.set_type(GameProto::EnterRoomSuccess);
                roomInfo &tempRoomInfo = roomMap[roomId];
      //  serverMsg.set_type(GameProto::BroadRoomInfo);
                GameProto::RoomInfo* roominfos = serverMsg.add_roominfos();
                roominfos->set_id(tempRoomInfo.rid);
                roominfos->set_roundtime(tempRoomInfo.roundTime);
                roominfos->set_roundnumber(tempRoomInfo.roundNumber);
                roominfos->set_roomname(tempRoomInfo.roomName);
                roominfos->set_password(tempRoomInfo.password);
                roominfos->set_mapname(tempRoomInfo.mapMame);
                roominfos->set_maxplayers(tempRoomInfo.rid);
                roominfos->set_master(tempRoomInfo.master);
                roominfos->set_curplayernumber(tempRoomInfo.preparedPlayers.size() + tempRoomInfo.unPreparedPlayers.size());
                printf("==========================\n !!!!!!===== %s  %d, %d\n", roominfos -> mapname().c_str() , roominfos->master(), tempRoomInfo.master);
                for(auto uid : tempRoomInfo.preparedPlayers) {
                // roominfos->add_preparedplayers(userInfoMap[uid]);
                    GameProto::PlayerInfo tempPlayerInfoProto;
                    playerInfo tempPlayerInfo = userInfoMap[uid];
                    tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
                    tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
                    tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
                    roominfos -> add_preparedplayers() -> CopyFrom(tempPlayerInfoProto);
                }
                for(auto uid : tempRoomInfo.unPreparedPlayers) {
                    //    roominfos->add_unpreparedplayers(uid);
                    GameProto::PlayerInfo tempPlayerInfoProto;
                    playerInfo tempPlayerInfo = userInfoMap[uid];
                    tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
                    tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
                    tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
                    roominfos -> add_unpreparedplayers() -> CopyFrom(tempPlayerInfoProto);
                }
            }
        }
    }
    printf(" =================!!===\n %s \n", serverMsg.roominfos(0).mapname().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
    if(serverMsg.type() == GameProto::EnterRoomSuccess) {
        roomInfo &tempRoomInfo = roomMap[roomId];
        serverMsg.set_type(GameProto::BroadRoomInfo);
        serverMsg.clear_roominfos();
        GameProto::RoomInfo* roominfos = serverMsg.add_roominfos();
        roominfos->set_id(tempRoomInfo.rid);
        roominfos->set_roundtime(tempRoomInfo.roundTime);
        roominfos->set_roundnumber(tempRoomInfo.roundNumber);
        roominfos->set_roomname(tempRoomInfo.roomName);
        roominfos->set_password(tempRoomInfo.password);
        roominfos->set_maxplayers(tempRoomInfo.rid);
        roominfos->set_master(tempRoomInfo.master);
        roominfos->set_curplayernumber(tempRoomInfo.preparedPlayers.size() + tempRoomInfo.unPreparedPlayers.size());
        for(auto uid : tempRoomInfo.preparedPlayers) {
            // roominfos->add_preparedplayers(userInfoMap[uid]);
            GameProto::PlayerInfo tempPlayerInfoProto;
            playerInfo tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
            tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
            tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
            roominfos -> add_preparedplayers() -> CopyFrom(tempPlayerInfoProto);
        }
        for(auto uid : tempRoomInfo.unPreparedPlayers) {
        //    roominfos->add_unpreparedplayers(uid);
            GameProto::PlayerInfo tempPlayerInfoProto;
            playerInfo tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
            tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
            tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
            roominfos -> add_unpreparedplayers() -> CopyFrom(tempPlayerInfoProto);
        }

        for(auto uid : tempRoomInfo.preparedPlayers) {
            HandleSendDataToClient(serverMsg, userFdMap[uid]);
        }
        for(auto uid : tempRoomInfo.unPreparedPlayers) {
            HandleSendDataToClient(serverMsg, userFdMap[uid]);
        }
    }
    BroadRoomInfo();
}
void hallServer::HandleExitRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    int roomId = clientMsg.roominfo().id(), uid;
    GameProto::ServerMsg serverMsg;
    if(fdUserMap.find(clientFd) == fdUserMap.end()) {
        //to do DontLogIn
    }else {
        uid = fdUserMap[clientFd];
        if(roomMap.find(roomId) == roomMap.end()) {
            //to do roomDontExist
        }else {
            roomInfo& tempRoomInfo = roomMap[roomId];
            if(tempRoomInfo.preparedPlayers.size()==0 && tempRoomInfo.unPreparedPlayers.size() == 0) {
                //to do RoomEmpty
            }else {
                tempRoomInfo.preparedPlayers.erase(uid);
                tempRoomInfo.unPreparedPlayers.erase(uid);
                if(uid == tempRoomInfo.master) {
                    if(tempRoomInfo.preparedPlayers.size() == 0) {
                        if(tempRoomInfo.unPreparedPlayers.size() == 0) {
                            tempRoomInfo.master = -1;
                        }else {
                            tempRoomInfo.master = *tempRoomInfo.unPreparedPlayers.begin();
                            tempRoomInfo.unPreparedPlayers.erase(tempRoomInfo.master);
                            tempRoomInfo.preparedPlayers.insert(tempRoomInfo.master);
                        }
                    }else {
                        tempRoomInfo.master = *tempRoomInfo.preparedPlayers.begin();
                    }
                }
            }
        }
    }
    HandleSendDataToClient(serverMsg, clientFd);
    if(serverMsg.type() == GameProto::ExitRoomSuccess) {
        roomInfo &tempRoomInfo = roomMap[roomId];
        serverMsg.set_type(GameProto::BroadRoomInfo);
        GameProto::RoomInfo* roominfos = serverMsg.add_roominfos();
        roominfos->set_id(tempRoomInfo.rid);
        roominfos->set_roundtime(tempRoomInfo.roundTime);
        roominfos->set_roundnumber(tempRoomInfo.roundNumber);
        roominfos->set_roomname(tempRoomInfo.roomName);
        roominfos->set_password(tempRoomInfo.password);
        roominfos->set_maxplayers(oneRoomMaxUsers);
        roominfos->set_master(tempRoomInfo.master);
        roominfos->set_curplayernumber(tempRoomInfo.preparedPlayers.size() + tempRoomInfo.unPreparedPlayers.size());
        for(auto uid : tempRoomInfo.preparedPlayers) {
            // roominfos->add_preparedplayers(userInfoMap[uid]);
            GameProto::PlayerInfo tempPlayerInfoProto;
            playerInfo tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
            tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
            tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
            roominfos -> add_preparedplayers() -> CopyFrom(tempPlayerInfoProto);
        }
        for(auto uid : tempRoomInfo.unPreparedPlayers) {
        //    roominfos->add_unpreparedplayers(uid);
            GameProto::PlayerInfo tempPlayerInfoProto;
            playerInfo tempPlayerInfo = userInfoMap[uid];
            tempPlayerInfoProto.set_uid(tempPlayerInfo.uid);
            tempPlayerInfoProto.set_roomid(tempPlayerInfo.rid);
            tempPlayerInfoProto.set_nickname(tempPlayerInfo.GetNickName());
            roominfos -> add_unpreparedplayers() -> CopyFrom(tempPlayerInfoProto);
        }

        for(auto uid : tempRoomInfo.preparedPlayers) {
            HandleSendDataToClient(serverMsg, userFdMap[uid]);
        }
        for(auto uid : tempRoomInfo.unPreparedPlayers) {
            HandleSendDataToClient(serverMsg, userFdMap[uid]);
        }
    }
    BroadRoomInfo();
}
void hallServer::HandleChangeStateInRoom(GameProto::ClientMsg clientMsg, int clientFd) {

}
void hallServer::HandleStartGame(GameProto::ClientMsg clientMsg, int clientFd) {
}
void hallServer::BroadRoomInfo() {
    GameProto::ServerMsg serverMsg;
    serverMsg.set_type(GameProto::BroadRoomListInfo);
    serverMsg.set_str("广播房间信息");
    GameProto::RoomInfo* roominfos;
    for(auto val : roomMap) {
        int idx = 0;
        roominfos = serverMsg.add_roominfos();
        roomInfo tempRoomInfo = val.second;
        roominfos->set_id(tempRoomInfo.rid);
        roominfos->set_roundtime(tempRoomInfo.roundTime);
        roominfos->set_roundnumber(tempRoomInfo.roundNumber);
        roominfos->set_roomname(tempRoomInfo.roomName);
        roominfos->set_password(tempRoomInfo.password);
        roominfos->set_mapname(tempRoomInfo.mapMame);
        roominfos->set_maxplayers(tempRoomInfo.maxPlayers);
        roominfos->set_master(tempRoomInfo.master);
        roominfos->set_curplayernumber(tempRoomInfo.preparedPlayers.size() + tempRoomInfo.unPreparedPlayers.size());
    }
    for(auto uid : onlineUsers) {
        HandleSendDataToClient(serverMsg, userFdMap[uid]);
    }
}

void hallServer::HandleSendDataToClient(GameProto::ServerMsg serverMsg, int clientFd) {
    if(clientFd == 0) {
        std::cout<< serverMsg.type() << std::endl;
        return ;
    }
    if(!serverMsg.SerializeToArray(bData.GetBuffArray() + HEAD_LENGTH, BUFF_SIZE)) {
        return ;
    }
    for(int i = 1; i < HEAD_LENGTH; ++i) {
        bData.ChangeBuffAt(serverMsg.ByteSize() >> ((i -1) * 8), i);
    }
    SendDataToClient(clientFd, HEAD_LENGTH + serverMsg.ByteSize());
}

void hallServer::HandleClose(int clientFd) {
    c2SDataMap.erase(clientFd);
    if(fdUserMap.find(clientFd) != fdUserMap.end()) {
        int clientUid = fdUserMap[clientFd];
        if(userRoomMap.find(clientUid) != userRoomMap.end()) {
            int clientRid = userRoomMap[clientUid];
            roomMap[clientRid].preparedPlayers.erase(clientUid);
            roomMap[clientRid].unPreparedPlayers.erase(clientUid);
            if(roomMap[clientRid].preparedPlayers.size() == 0 && roomMap[clientRid].unPreparedPlayers.size() == 0) {
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

