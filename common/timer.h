#pragma once
#include "heads.h"

class TriggerEvent {
public:
    long GetStartTime();
    void Action();
private:
    long startTime;
    std::function<void()> action;
};
