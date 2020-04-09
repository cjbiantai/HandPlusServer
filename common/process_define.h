// #ifndef __PROCESS_DEFINE_H
// #define __PROCESS_DEFINE_H
#pragma once
#define PROCESS_LOGFILE_PREFIX "hall_server"

#define STAT_FILE_HEAD "====================================================================================\n"
#define STACK_HEAD "\n==========================================\n"

enum
{
    MAX_STAT_LEN = 8192,
    MAX_POPEN_BUF_LEN = 10240,
    MAX_STACK_LEN = 204800,
    MAX_CONFIG_VALUE_LEN = 2048,
};

enum enmRunFlag
{
    run_flag_reload		= 0x00000001,
    run_flag_exit		= 0x00000002,
};

struct stSocketStatistics
{
    unsigned int m_unSocketCount;
    unsigned int m_unSocketAdd;
};

// #endif
