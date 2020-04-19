#pragma once
#include "../common/heads.h"

class service_mgr
{
public:
    service_mgr(std::string, int, int);
    bool operator < (const service_mgr&)const;
    std::string serviceIp;  //服务器IP
    int servicePort;    //服务器端口
    int servicePressure;    //该服务器负载
    int serviceId;
};

