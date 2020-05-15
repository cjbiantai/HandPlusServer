#include"log_manager.h"

FILE *LogManager::file=NULL;
int LogManager::level=0;
int LogManager::mode=(DEBUG_MODE_STDOUT);

void LogManager::Init(int level,int mode,const char *file_name){
    LogManager::level=level;
    LogManager::mode=mode;
    file=fopen(file_name,"ab");
    if(file==NULL){
        printf("LogManager open file fail!\n");
        exit(0);
    }
}

void LogManager::Log(int level,const char *str,...){
    if(level>LogManager::level)
        return;
    va_list args;
    va_start(args,str);
    if(mode&DEBUG_MODE_FILE){
        vfprintf(LogManager::file,str,args);
        fprintf(LogManager::file,"\n");
        fflush(LogManager::file);
    }
    va_start(args,str);
    if(mode&DEBUG_MODE_STDOUT){
        vprintf(str,args);
        cout<<endl;
    }
    va_end(args);
}

void LogManager::DetailLog(int level,const char *date,const char *file,int line,const char *str,...){
    if(level>LogManager::level)
        return;
    va_list args;
    va_start(args,str);
    if(mode&DEBUG_MODE_FILE){
        fprintf(LogManager::file,"%s, in file %s, line %d: ",date,file,line);
        vfprintf(LogManager::file,str,args);
        fprintf(LogManager::file,"\n");
        fflush(LogManager::file);
    }
    va_start(args,str);
    if(mode&DEBUG_MODE_STDOUT){
        printf("%s, in file: %s, line %d: ",date,file,line);
        vprintf(str,args);
        cout<<endl;
    }
    va_end(args);
}

void LogManager::Close(){
    fclose(LogManager::file);
}
