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
    std::string GetCharacterName() {return characterName;}

    void SetCharacterName(std::string characterName) {this->characterName = characterName;}

    void SetUid(int uid) {this->uid = uid;}

    void SetRid(int rid) {this->rid = rid;}

    int uid,rid;
private:
    std::string account;
    std::string nickName;
    std::string characterName;
};

