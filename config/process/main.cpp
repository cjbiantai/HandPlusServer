#include <cstdio>
#include <cstring>
#include "config.h"

#define CONFIG  "./config/process.xml"

Config config;


bool load_config(){
    if (true != config.LoadConfig(CONFIG)) {
        printf("failed loadconfig\n");
        return false;
    }
    else {
        printf("success loadconfig\n");
        return true;
    }
}

int main(){
    load_config();
    printf("[m_iLogLevel=%d]\n",config.m_iLogLevel);
    printf("[test_int=%d][test_string=%s]",config.test_int,config.test_string.c_str());
    for(auto v : config.test){
        printf("[name=%s][age=%d]\n",v.name.c_str(),v.age);
    }
    return 0;
}
