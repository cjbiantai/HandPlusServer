#pragma once
#include "consts.h"
#include "singleton.h"

class IdGenerator {
public:
    IdGenerator() {
        uid = 0;
        rid = 0;
    }
    uint GetUid() {return ++uid;}
    uint GetRid() {return ++rid;}
private:
    uint uid;
    uint rid;
};

typedef Singleton<IdGenerator> idGenerator;

#define GetUid() \
    idGenerator::Instance().GetUid()
#define GetRid() \
    idGenerator::Instance().GetRid()
