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

    void GetProto(GameProto::RoomInfo &roominfo) {
        roominfo.set_roomname(roomName);
        roominfo.set_roomid(rid);
        roominfo.set_roundtime(roundTime);
        roominfo.set_roundnumber(roundNumber);
        roominfo.set_mapname(mapName);
        roominfo.set_mapidx(mapIdx);
        roominfo.set_password(password);
        roominfo.set_maxplayers(maxPlayers);
        roominfo.set_masteruid(master);
        roominfo.set_curplayernumber(Players.size());
    }

    void SetFromProto(GameProto::RoomInfo roominfo) {
        roundTime = roominfo.roundtime();
        roundNumber = roominfo.roundnumber();
        roomName = roominfo.roomname();
        mapName = roominfo.mapname();
        mapIdx = roominfo.mapidx();
        password = roominfo.password();
        maxPlayers = roominfo.maxplayers();
    }

    int rid;                                 //房间Id
    int roundTime;                              //单局时间
    int roundNumber;                            //局数
    std::string roomName;                       //房间名
    std::string mapName;                        //地图名称
    int mapIdx;
    std::string password;                       //房间密码
    int maxPlayers;                             //最大玩家数
    std::string serviceAddr;                    //房间的服务器地址
    std::set<int> Players;              //准备中的玩家uid集合
    int master;                                 //房主uid
    int port;
    std::string ip;
};

