#include "config.h"
#include <cstring>

#define GetInt32Attribute(key, node) \
if (!key || !node){\
pszValue = NULL; \
} \
else{\
rapidxml::xml_attribute<>* attr = node->first_attribute(key); \
pszValue = attr ? attr->value() : NULL; \
if (pszValue && strlen(pszValue) > 0){\
    iValue = atoi(pszValue); \
}\
}

#define GetStringAttrbute(key, node) \
if (!key || !node){\
pszValue = NULL; \
}\
else{ \
rapidxml::xml_attribute<>* attr = node->first_attribute(key); \
pszValue = attr ? attr->value() : NULL; \
}

#define STRNCPY(dst, src, length)					      \
	do								      \
	{								      \
		strncpy(dst, src, (length-1));			      \
		dst[(length-1)] = '\0';					      \
	}								      \
	while(0)


template <typename T>
void Config::ParseIntegerValue(rapidxml::xml_node<>* node, const char* key, T& ret, T default_value){
    GetInt32Attribute(key, node);
    if (NULL == pszValue) {
        ret = default_value;
    } else {
        ret = iValue;
    }
    printf("[Config][key=%s][value=%d]\n", key, ret);
}

void Config::ParseDecimalValue(rapidxml::xml_node<>* node, const char* key, double& ret, double default_value){
    GetStringAttrbute(key, node);
    if (NULL == pszValue) {
        ret = default_value;
    } else {
        memset(m_szTmp, 0, sizeof(m_szTmp));
        STRNCPY(m_szTmp, pszValue, sizeof(m_szTmp));
        ret = strtod(m_szTmp, NULL);
    }
    printf("[Config][key=%s][value=%f]\n", key, ret);
}

void Config::ParseStringValue(rapidxml::xml_node<>* node, const char* key, std::string& ret, std::string default_value){
    GetStringAttrbute(key, node);
    if (NULL == pszValue) {
        ret = default_value;
    } else {
        memset(m_szTmp, 0, sizeof(m_szTmp));
        STRNCPY(m_szTmp, pszValue, sizeof(m_szTmp));
        ret = std::string(m_szTmp);
    }
    printf("[Config][key=%s][value=%s]\n", key, ret.c_str());
}

bool Config::LoadConfig(const char* pszFileName)
{
    rapidxml::file<> stFile(pszFileName);
    if (!stFile.size())
    {
        printf("LoadFile(%s) error!\n", pszFileName);
        return false;
    }
    rapidxml::xml_document<> stDoc;
    stDoc.parse<0>(stFile.data());
    
    rapidxml::xml_node<>* node = stDoc.first_node("process"); //看这里。。第一个节点 配合配置文件看
    if (NULL == node)
    {
        printf("LoadFile(%s) do not have (process) node!\n", pszFileName);
        return false;
    }
    rapidxml::xml_node<>* childNode = NULL;
    rapidxml::xml_node<>* grandNode = NULL;  

    ParseIntegerValue(node, "hall_port", hallPort, -1); //在看这里 然后往下面看就懂了。。
                //            配置文件字段名/要赋值的变量/   默认的值
                //
    childNode = node->first_node("sql_table");
    ParseStringValue(childNode, "table_name", tableName, "");
    
    childNode = node->first_node("room");
    ParseIntegerValue(childNode, "room_number", roomconfig.roomNumber, -1);
    ParseIntegerValue(childNode, "room_max_user", roomconfig.roomMaxUser, -1);


    //下面是数组的示例
    childNode = node->first_node("service_array");
    serviceConfigs.clear();
    while(childNode != nullptr){
        serviceConfig tmpServiceConfig;
        ParseStringValue(childNode, "ip", tmpServiceConfig.ip, "");
        ParseIntegerValue(childNode, "port", tmpServiceConfig.port, -1);
        serviceConfigs.push_back(tmpServiceConfig);
        childNode = childNode ->next_sibling("service_array");
    } 
    
    return true;
}
