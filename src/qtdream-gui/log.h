#pragma once

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include <stdlib.h>

#include <QString>

static log4cpp::Category & LogCategory = log4cpp::Category::getInstance("Category");

#define LOG_EMERG  Log(log4cpp::Priority::EMERG,  true)
#define LOG_FATAL  Log(log4cpp::Priority::FATAL,  true)
#define LOG_ALERT  Log(log4cpp::Priority::ALERT,  false)
#define LOG_CRIT   Log(log4cpp::Priority::CRIT,   false)
#define LOG_ERROR  Log(log4cpp::Priority::ERROR,  false)
#define LOG_WARN   Log(log4cpp::Priority::WARN,   false)
#define LOG_NOTICE Log(log4cpp::Priority::NOTICE, false)
#define LOG_INFO   Log(log4cpp::Priority::INFO,   false)
#define LOG_DEBUG  Log(log4cpp::Priority::DEBUG,  false)

class Log
{
public:
    Log(log4cpp::Priority::Value level, bool doHalt) : level_(level), doHalt_(doHalt) {}

    void operator()(const QString & str) const {
        LogCategory.log(level_, str.toStdString());
        if(doHalt_) ::abort();
    }

    void operator()(const char * str) const {
        LogCategory.log(level_, str);
        if(doHalt_) ::abort();
    }

    template<typename T1>
    void operator()(const QString & str, const T1 & t1) const {
        LogCategory.log(level_, QString(str).arg(t1).toStdString());
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2) const {
        LogCategory.log(level_, QString(str).arg(t1).arg(t2).toStdString());
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3) const {
        LogCategory.log(level_, QString(str).arg(t1).arg(t2).arg(t3).toStdString());
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3, const T4 & t4) const {
        LogCategory.log(level_, QString(str).arg(t1).arg(t2).arg(t3).arg(t4).toStdString());
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3, const T4 & t4, const T5 & t5) const {
        LogCategory.log(level_, QString(str).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5).toStdString());
        if(doHalt_) ::abort();
    }

private:
    log4cpp::Priority::Value level_;
    bool                     doHalt_;
};
