#pragma once
#include "../common/heads.h"

class roomInfo
{
public:
    roomInfo() {}
    ~roomInfo(){}
    int roomId; //房间Id
    std::string serviceAddr; //房间的服务器地址
    std::set<int> userSet;  //当前房间用户的集合
};

