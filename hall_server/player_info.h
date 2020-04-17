#pragma once
#include "../common/heads.h"
#include "../common/xid.cc"

class playerInfo
{
public:
    playerInfo() {}
    playerInfo(std::string account, std::string nickName) {
        this->account = account;
        this->nickName = nickName;
        uid = GetUid();
        rid = -1;
    }
    ~playerInfo() {}

    std::string GetAccount() {
        return account;
    }

    std::string GetNickName() {return nickName;}
    int GetUid() {return uid;}
    void SetUid(int uid) {this->uid = uid;}

    int GetRid() {return rid;}
    void SetRid(int rid) {this->rid = rid;}



private:
    int uid;
    std::string account;
    std::string nickName;
    int rid;
};

