
#ifndef QKRTL_LOGGER_H
#define QKRTL_LOGGER_H 1

#include <time.h>
#include <string>
#include <mutex>

#include "qkrtl/Compile.h"
#include "qkrtl/DateTime.h"

namespace qkrtl {

/**
    暂停升级，未来加入异步写盘功能
*/
class Logger {
public:
    QKRTLAPI Logger();
    QKRTLAPI virtual ~Logger();
    QKRTLAPI bool init(const std::string& name);
    QKRTLAPI int write(const char *file_name, int line, int level, const char *format, ...);

    inline int fileLevel() const { return fileLevel_; }
    inline void fileLevel(int level) { fileLevel_ = level; }
    inline int ttyLevel() const { return ttyLevel_; }
    inline void ttyLevel(int level) { ttyLevel_ = level; }

    static const int kEMERG = 0;
    static const int kALERT = 1;
    static const int kCRIT = 2;
    static const int kERROR = 3;
    static const int kWARN = 4;
    static const int kNOTICE = 5;
    static const int kINFO = 6;
    static const int kDEBUG = 7;
    static const int kMAXLEVEL = kDEBUG;

private:
    FILE * file_;
    int fileLevel_;
    int ttyLevel_;
    std::string name_;
    Date today_;
    std::mutex guard_;
    void close();
    void open();
    void writeMessage(const char * message);
};
}

#ifdef __cplusplus
extern "C" {
#endif

QKRTLAPI qkrtl::Logger * GetDefaltLogger();
QKRTLAPI void SetDefaltLogger(const char * name);

#ifdef __cplusplus
}
#endif

#define LOGWRITE(level , format , ...) \
	do{									\
		if(GetDefaltLogger()->fileLevel() < level) \
			break ;								\
		GetDefaltLogger()->write(__FILE__ , __LINE__ , level , format , ##__VA_ARGS__);	\
	}while(0)

#define LOGEMERG(format , ...) LOGWRITE(qkrtl::Logger::kEMERG , format , ##__VA_ARGS__) 
#define LOGALERT(format , ...) LOGWRITE(qkrtl::Logger::kALERT , format , ##__VA_ARGS__) 
#define LOGCRIT(format , ...) LOGWRITE(qkrtl::Logger::kCRIT , format , ##__VA_ARGS__) 
#define LOGERR(format , ...) LOGWRITE(qkrtl::Logger::kERROR , format , ##__VA_ARGS__) 
#define LOGWARN(format , ...) LOGWRITE(qkrtl::Logger::kWARN , format , ##__VA_ARGS__) 
#define LOGNOTICE(format , ...) LOGWRITE(qkrtl::Logger::kNOTICE , format , ##__VA_ARGS__) 
#define LOGINFO(format , ...) LOGWRITE(qkrtl::Logger::kINFO , format , ##__VA_ARGS__) 
#define LOGDEBUG(format , ...) LOGWRITE(qkrtl::Logger::kDEBUG , format , ##__VA_ARGS__) 

#endif /**QKRTL_LOGGER_H*/
