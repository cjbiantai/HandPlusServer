#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "../common/rapidxml.hpp"
#include "../common/rapidxml_print.hpp"
#include "../common/rapidxml_utils.hpp"
#include <string>
#include <vector>

struct Test{
    std::string name;
    int32_t age;
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
    int32_t  m_iLogLevel;
    int32_t  test_int;
    std::string test_string;
    std::vector<Test> test;
};

#endif