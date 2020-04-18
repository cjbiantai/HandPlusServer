#pragma once
#include "../common/heads.h"
#include "../common/xid.cc"

class roomInfo
{
public:
    roomInfo() {
        rid = GetRid();
    }
    ~roomInfo(){}
    int rid;                                 //房间Id
    int roundTime;                              //单局时间
    int roundNumber;                            //局数
    std::string roomName;                       //房间名
    std::string mapMame;                        //地图名称
    std::string password;                       //房间密码
    int maxPlayers;                             //最大玩家数
    std::string serviceAddr;                    //房间的服务器地址
    std::set<int> preparedPlayers;              //准备中的玩家uid集合
    std::set<int> unPreparedPlayers;            //未准备的玩家
    int master;                                 //房主uid
};

