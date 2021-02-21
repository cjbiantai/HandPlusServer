#include <stdio.h>
#include <mysql/mysql.h>

#define HOST "127.0.0.1"
#define USERNAME "root"
#define PASSWORD "88888888"
#define DATABASE "test"
#define TABLENAME "USER"

#define RE_LOG_IN -3
#define CONNECT_TO_SQL_ERROR -2
#define QUERY_SQL_ERROR -1
#define QUERY_EMPTY 1
#define QUERY_OK 0

int query_sql(char* sql);

