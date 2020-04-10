#pragma once
#include "../server_base/server_base.h"
#include "../common/heads.h"
#include "../common/consts.h"
#include "../common/mysql/mysql.h"
#include "process_log.h"
#include "../common/game_proto.pb.h"
#include "../common/recv_data_manager.h"
#include "service_mgr.h"
#include "room_info.h"
#include "config.h"

//大厅服务器进程，处理登录,注册，房间等事件
class hallServer : serverBase {
public:
    /**
     * @brief 构造函数，通过配置文件进行初始化
     * @param config 配置文件信息，用来初始化数据库表名，房间信息以及服务器转发信息
     */
    hallServer(Config config);
    ~hallServer(){}

    /**
     * @brief 处理具体大厅逻辑,处理沾包
     * @param clientFd 客户端fd
     * @param dataLength 服务器recv到的数据的长度
     */
    void HandleEvent(int clientFd, int dataLength); 
    /**
     * @brief 处理连接关闭时的事件
     * @param clientFd 客户端fd
     */
    void HandleClose(int clientFd);  
    /**
     * @brief 处理登录事件
     * @param clientMsg 反序列化的客户端发送的数据
     * @param clientFd 客户端fd
     */
    void HandleLogIn(GameProto::ClientMsg clientMsg, int clientFd);
    /**
     * @brief 注册登录事件
     * @param clientMsg 反序列化的客户端发送的数据
     * @param clientFd 客户端fd
     */
    void HandleRegist(GameProto::ClientMsg clientMsg, int clientFd);
    /**
     * @brief 处理进入房间事件
     * @param clientMsg 反序列化的客户端发送的数据
     * @param clientFd 客户端fd
     */
    void HandleSelectRoom(GameProto::ClientMsg clientMsg, int clientFd);
    /*
     * @brief 处理发送数据给单个客户端的情况
     * @param serverMsg 未序列化的发给客户端的数据
     * @param clientFd 客户端fd
     */
    void HandleSendDataToClient(GameProto::ServerMsg serverMsg, int clientFd);
    /*
     * @brief 广播房间信息, 仅当房间信息改变了才会调用
     */
    void BroadRoomInfo();
    void Work();    

private:
    std::string tableName; //数据库表名
    int oneRoomMaxUsers, roomNumber, servicePressureLimit;    //单个房间最大的用户数量, 房间的最大数量, 服务器最大负载
    std::map<int, recvDataManager> c2SDataMap; //clientfd对应的缓冲区，处理沾包
    std::map<int, std::string> fdUserMap;   //clientfd到用户名的映射
    std::map<int, roomInfo> roomMap;    //房间id到房间信息的映射
    std::vector<service_mgr> serviceList;  //存放战斗服务器信息
    std::set<std::string> onlineUsers;  //在线用户用户名集合
    std::set<int> onlineClients;    //在线clientfd集合
    std::set<int> hallClients; //在大厅的clientfd的集合 
};

