
#include "qkrtl/Logger.h"
#include "qkrtl/ThreadHelper.h"

namespace qkrtl {

const char *TrimFileName(const char *file_name)
{
    const char *pstr;

    if (file_name == NULL)
        return NULL;

    pstr = file_name + strlen(file_name);
    while (pstr != file_name)
    {
        if (*pstr == '/' || *pstr == '\\')
        {
            pstr++;
            break;
        }
        pstr--;
    }

    return pstr;
}

const char *__LogLevelName__[8] =
{
    "emerg",
    "alert",
    "crit",
    "error",
    "warn",
    "notic",
    "info",
    "debug"
};

void Logger::writeMessage(const char * message)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (file_ == NULL || message == NULL)
        return;

    int fsize = fprintf(file_, "%s \n", message);
    if (fsize > 0)
    {
        fflush(file_);
    }
}

Logger::Logger():file_(NULL) , fileLevel_(kERROR) , ttyLevel_(kEMERG)
{
    //
}
Logger::~Logger()
{
    close();
}
bool Logger::init(const std::string& name)
{
    if (name_.empty() == true)
        name_ = "LOG";
    else
        name_ = name;
    return true;
}
int Logger::write(const char *file_name, int line, int level, const char *format, ...)
{
    static const int kMaxLogLength = 4096;
    va_list ap;
    char message[kMaxLogLength] = { '\0' }, *pos;
    int len = 0;

    if (level > fileLevel_)
        return 0;

    //实现日志按每日存放的功能！
    if (today_ != Date::today())
    {
        close();
        open();
    }

    uint32_t threadId = GetCurrentThreadId();
    DateTime now(UTimeNow());
    if (level >= 0 && level <= kMAXLEVEL)
    {
        len = sprintf(message, "%s [%s] TID[%u] FILE[%s]LINE[%u] ",
            now.toString().c_str(), __LogLevelName__[level], threadId,
            TrimFileName(file_name), line);
    }
    else
    {
        len = sprintf(message, "%s TID[%u] FILE[%s]LINE[%d] ",
            now.toString().c_str(), threadId,
            TrimFileName(file_name), line);
    }

    pos = message + len;
    len = kMaxLogLength - len - 30;

    va_start(ap, format);
    vsnprintf(pos, len, format, ap);
    va_end(ap);

    writeMessage(message);

    if (ttyLevel_ >= level)
    {
        printf("%s \n", message);
    }

    return 0;
}
void Logger::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (file_)
    {
        fflush(file_);
        fclose(file_);

        file_ = NULL;
    }
}
void Logger::open()
{
    char fileName[256] = {'\0'};
    today_.calcNow();
    snprintf(fileName ,sizeof(fileName), "%s%s.log", name_.c_str(), today_.toString().c_str());

    std::unique_lock<std::mutex> locker(guard_);
    if (file_ != NULL)
        return;

    file_ = fopen(fileName, "w+b");
}
}
class LoggerInitializer {
public:
    LoggerInitializer() : defaultLogger_(NULL) {}
    virtual ~LoggerInitializer()
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (defaultLogger_ != NULL)
        {
            delete defaultLogger_;
            defaultLogger_ = NULL;
        }
    }

    qkrtl::Logger * defaultLogger()
    {
        if (defaultLogger_ == NULL)
        {
            std::unique_lock<std::mutex> locker(guard_);
            defaultLogger_ = new qkrtl::Logger();
            defaultLogger_->init("");
        }
        return defaultLogger_;

    }
    void init(const char * name)
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (defaultLogger_ != NULL)
            return;
        defaultLogger_ = new qkrtl::Logger();
        if (name == NULL)
            defaultLogger_->init("");
        else
            defaultLogger_->init(name);
    }

private:
    std::mutex guard_;
    qkrtl::Logger * defaultLogger_;
};

static LoggerInitializer __LoggerInitializer__;

qkrtl::Logger * GetDefaltLogger()
{
    return __LoggerInitializer__.defaultLogger();
}
void SetDefaltLogger(const char * name)
{
    __LoggerInitializer__.init(name);
}

