#pragma once
#include "../common/heads.h"

class roomInfo
{
public:
    roomInfo();
    ~roomInfo(){}
    int roomId; //房间Id
    int serviceId; //该房间的服务器Id
    std::set<int> userSet;  //当前房间用户的集合
};

