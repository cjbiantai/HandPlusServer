#pragma once
static unsigned int uid = 0, rid = 0;

static unsigned int GetUid() {
    return ++uid;
}

static unsigned int GetRid() {
    return ++rid;
}
