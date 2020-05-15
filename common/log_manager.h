#pragma once
#include<iostream>
#include<stdarg.h>
#include<fstream>
#include<cstdio>
using namespace std;

#define DEBUG_MODE_STDOUT 1
#define DEBUG_MODE_FILE 2
#define LOG(level,str,...) LogManager::Log(level,str,##__VA_ARGS__)
#define DETAILLOG(level,str,...) LogManager::DetailLog(level,__DATE__,__FILE__,__LINE__,str,##__VA_ARGS__)

class LogManager{
    static FILE *file;
    public:
        static int level;
        static int mode;
        static void Init(int level,int mode,const char *file_name="log");
        static void Log(int level,const char *str,...);
        static void DetailLog(int level,const char *date,const char *file,int line,const char *str,...);
        static void Close();
};

