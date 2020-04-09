#include "hall_server.h"

hallServer::hallServer() : serverBase(HALL_PORT) {
}

void hallServer::HandleEvent(int clientFd, int dataLength) {
    if(c2SDataMap.find(clientFd) == c2SDataMap.end()) {
        c2SDataMap[clientFd] = RecvDataManager();    
    }
    RecvDataManager &recvDataManager = c2SDataMap[clientFd];
    GameProto::ClientMsg clientMsg;
    for(int i = 0; i < dataLength; ++i) {
        recvDataManager.PushByte((byte)bData.GetBuffCharAt(i));
    }
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
    std::string sqlQuery = "select * from " + TABLENAME + " where name = '" + clientMsg.name() + "' and password = '" + clientMsg.password() + "';";
    int ret = query_sql(sqlQuery.c_str());
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
            serverMsg.set_code(0);
            serverMsg.set_str("登录成功");
            //登录成功
            break;
    }
    printf("%s\n", serverMsg.str().c_str());
}

void hallServer::HandleRegist(GameProto::ClientMsg clientMsg, int clientFd) {
    GameProto::ServerMsg serverMsg;
    std::string sqlQuery = "insert into " + TABLENAME + " values('" + clientMsg.name() + "', '" + clientMsg.password() + "');";
    printf("%s\n", sqlQuery.c_str());
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
}

void hallServer::HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd) {
    
}
std::string hallServer::GetServer() {
    return SERVER_IP + std::to_string(SERVER_PORTS[0]);
}
void hallServer::HandleClose(int clientFd) {
    printf("---->_<----!!!\n");

}

void hallServer::Work() {
    serverBase::Work();
}

