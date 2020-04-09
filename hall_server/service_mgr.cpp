#include "service_mgr.h"

service_mgr::service_mgr(std::string serviceIp, int servicePort, int serviceId) {
    this -> serviceIp = serviceIp;
    this -> servicePort = servicePort;
    this -> serviceId = serviceId;
}

bool service_mgr::operator<(const service_mgr service) {
    return this -> servicePressure > service.servicePressure;
}
