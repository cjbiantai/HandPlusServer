
#pragma once
#include <iostream>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
enum
{
	success = 0,
	fail = 1,
};


enum
{
	max_datetime_string_length = 32,  /**<日期时间字符串的最大长度 <*/
	max_file_name_length = 255,		  /**<文件名最大长度<*/
	max_path_length = 255,			  /**<路径的最大长度*/ 
};

