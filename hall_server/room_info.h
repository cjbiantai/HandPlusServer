#pragma once
#include "../common/heads.h"

class roomInfo
{
public:
    roomInfo();
    ~roomInfo(){}

private:
    int roomId; //房间Id
    int limit; //房间用户上限
    std::set<int> userSet;  //当前房间用户的集合
};

