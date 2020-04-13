#include "hall_server.h"

hallServer::hallServer(Config config) : serverBase(config.hallPort) {
    for(int i = 0; i < config.serviceConfigs.size(); ++i) {
        serviceConfig tmpServiceConfig = config.serviceConfigs[i];
        serviceList.push_back(service_mgr(tmpServiceConfig.ip, tmpServiceConfig.port, i+1));
    }
    oneRoomMaxUsers = config.roomconfig.roomMaxUser;
    roomNumber = config.roomconfig.roomNumber;
    for(int i = 0; i < roomNumber; ++i) {
        roomMap[i+1] = roomInfo();
        roomMap[i+1].roomId = i + 1;
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
    GameProto::ClientMsg clientMsg;
    while(true) {
        int packageLength = recvDataManager.GetPackageLength();
        if(packageLength == -1) {
            break;
        }
        for(int i = 0; i < HEAD_LENGTH; ++i) {
            recvDataManager.PopByte();
        }
        for(int i = 0; i < packageLength; ++i) {
            bData.ChangeDataAt(recvDataManager.PopByte(), i);
        }
        int ret = clientMsg.ParseFromArray(bData.GetDataArray(), packageLength);
        switch(clientMsg.type()) {
            case 0 :
                HandleLogIn(clientMsg, clientFd);
                break;
            case 1 :
                HandleRegist(clientMsg, clientFd);
                break;
            case 2 :
                HandleSelectRoom(clientMsg, clientFd);
                break;
        }
        break;
    }
}
void hallServer::HandleLogIn(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string sqlQuery = "select * from " + tableName + " where name = '" + clientMsg.name() + "' and password = '" + clientMsg.password() + "';";
    int ret = query_sql(sqlQuery.c_str());
    printf("%s\n", sqlQuery.c_str());
    switch(ret) {
        case CONNECT_TO_SQL_ERROR:
            serverMsg.set_code(1);
            serverMsg.set_str("服务器出错了");
            //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_code(1);
            serverMsg.set_str("服务器出错了");
            //数据库语句出错
            break;
        case QUERY_EMPTY:
            serverMsg.set_code(1);
            serverMsg.set_str("账号或密码错误");
            //用户账号或密码错误
            break;
        case QUERY_OK:
            if(onlineUsers.find(clientMsg.name()) != onlineUsers.end()) {
                serverMsg.set_code(1);
                serverMsg.set_str("用户已登录");
                break;
            }
            fdUserMap[clientFd] = clientMsg.name();
            onlineUsers.insert(clientMsg.name());
            onlineClients.insert(clientFd);
            serverMsg.set_code(0);
            if(userHostMap.find(clientMsg.name()) != userHostMap.end()) {
                serverMsg.set_str(userHostMap[clientMsg.name()]);
            }else {
                serverMsg.set_str("登录成功");
            }
            BroadRoomInfo();
            //登录成功
            break;
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
}

void hallServer::HandleRegist(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string sqlQuery = "insert into " + tableName + " values('" + clientMsg.name() + "', '" + clientMsg.password() + "');";
    int ret = query_sql(sqlQuery.c_str());   
    switch(ret) {
        case CONNECT_TO_SQL_ERROR :
            serverMsg.set_code(1);
            serverMsg.set_str("服务器出错了");
            //连接数据库出错
            break;
        case QUERY_SQL_ERROR:
            serverMsg.set_code(1);
            serverMsg.set_str("用户已存在");
            //用户已存在
            break;
        default:
            serverMsg.set_code(0);
            serverMsg.set_str("注册成功");
            //注册成功
    }
    printf("%s\n", serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd);
}

void hallServer::HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    roomInfo &roominfo = roomMap[clientMsg.id()];
    if(onlineUsers.find(clientMsg.name()) == onlineUsers.end()) {
        serverMsg.set_code(1);
        serverMsg.set_str("请先登录");
    }else  if(roominfo.userSet.size() == oneRoomMaxUsers) {
        serverMsg.set_code(1);
        serverMsg.set_str("房间人数已满");
    }else {
        roominfo.userSet.insert(clientFd);
        serverMsg.set_code(0);
        serverMsg.set_str("进入房间成功");
        //hallClients.erase(clientFd);
    }
    printf("room: %s %d\n", clientMsg.name().c_str(), clientMsg.id());
    printf("%d %s\n", serverMsg.code(), serverMsg.str().c_str());
    HandleSendDataToClient(serverMsg, clientFd); 
    if(roominfo.userSet.size() == oneRoomMaxUsers && serverMsg.code() == 0) {
        int minPressure = servicePressureLimit - oneRoomMaxUsers, serviceId = -1;
        for(int i = 0; i < serviceList.size(); ++i) {
            if(serviceList[i].servicePressure <= minPressure) {
                serviceId = i;
                minPressure = serviceList[i].servicePressure;
            }
        }
        if(serviceId == -1) {
            serverMsg.set_code(1);
            serverMsg.set_str("服务器满了 ！！！\n");
        }else {
            serverMsg.set_code(0);
            roominfo.serviceId = serviceId;
            userHostMap[clientMsg.name()] = serviceList[serviceId].serviceIp + ":"+ std::to_string(serviceList[serviceId].servicePort);
            serverMsg.set_str(userHostMap[clientMsg.name()]);
            serviceList[serviceId].servicePressure += oneRoomMaxUsers;
        }
        printf("%d %s\n", serverMsg.code(), serverMsg.str().c_str());
    
        for(auto fd : roominfo.userSet) {
            HandleSendDataToClient(serverMsg, fd);
        }
    }

    BroadRoomInfo();    
}

void hallServer::BroadRoomInfo() {
    GameProto::ServerMsg serverMsg;
    serverMsg.set_code(3);
    GameProto::RoomInfo* roominfo;
    for(int i = 0; i < roomNumber; ++i) {
        roominfo = serverMsg.add_roominfos();
        roominfo -> set_id(i+1);
        roominfo -> set_maxusers(oneRoomMaxUsers);
        for(auto fd : roomMap[i+1].userSet) {
            roominfo -> add_members(fdUserMap[fd]);
        }
    }
    for(auto fd : onlineClients) {
        HandleSendDataToClient(serverMsg, fd);
    }
}

void hallServer::HandleSendDataToClient(GameProto::ServerMsg serverMsg, int clientFd) {
    if(!serverMsg.SerializeToArray(bData.GetBuffArray() + HEAD_LENGTH, BUFF_SIZE)) {
        return ;
    }
    for(int i = 1; i < HEAD_LENGTH; ++i) {
        bData.ChangeBuffAt(serverMsg.ByteSize() >> ((i -1) * 8), i);
    }
    SendDataToClient(clientFd, HEAD_LENGTH + serverMsg.ByteSize());
}

void hallServer::HandleClose(int clientFd) {
    for(auto room : roomMap) {
        room.second.userSet.erase(clientFd);
    }
    c2SDataMap.erase(clientFd);
    onlineUsers.erase(fdUserMap[clientFd]);
    fdUserMap.erase(clientFd);
    onlineClients.erase(clientFd);
}

void hallServer::Work() {
    serverBase::Work();
}

