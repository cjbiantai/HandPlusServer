#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "../common/xml/rapidxml.hpp"
#include "../common/xml/rapidxml_print.hpp"
#include "../common/xml/rapidxml_utils.hpp"
#include "service_mgr.h"
#include <string>
#include <vector>

struct serviceConfig {
    std::string ip;
    int port;
};
class Config{
public:
    bool LoadConfig(const char* pszFileName);
	template <typename T>
	void ParseIntegerValue(rapidxml::xml_node<>* node, const char* key, T& ret, T default_value);
	void ParseDecimalValue(rapidxml::xml_node<>* node, const char* key, double& ret, double default_value);
	void ParseStringValue(rapidxml::xml_node<>* node, const char* key, std::string& ret, std::string default_value);
public:
    const char *pszValue = NULL;
	int32_t iValue = 0;
	char m_szTmp[2048];

public:
    int hallPort,servicePressureLimit;
    std::string tableName;
    std::vector<serviceConfig> serviceConfigs;
};

#endif
