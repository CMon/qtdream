#pragma once

#include <stdlib.h>
#include <QDesktopServices>
#include <QDir>
#include <QString>
#include <QDateTime>

#include <qdebug.h>

#define LOG_FATAL  Log(5, true,  __FILE__, __LINE__)
#define LOG_ERROR  Log(4, false, __FILE__, __LINE__)
#define LOG_WARN   Log(3, false, __FILE__, __LINE__)
#define LOG_NOTICE Log(2, false, __FILE__, __LINE__)
#define LOG_INFO   Log(1, false, __FILE__, __LINE__)
#define LOG_DEBUG  Log(0, false, __FILE__, __LINE__)

#define LOG_DEBUG_FUNCTION ScopedFunctionLogger SCF(__FUNCTION__)

class Log
{
public:
    Log(int level, bool doHalt, const char * function, int line) :
        level_(level),
        doHalt_(doHalt),
        file_(function),
        line_(line)
    {
    }

    void operator()(const QString & str) const {
        logImpl(str);
        if(doHalt_) ::abort();
    }

    template<typename T1>
    void operator()(const QString & str, const T1 & t1) const {
        logImpl(QString(str).arg(t1));
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2) const {
        logImpl(QString(str).arg(t1).arg(t2));
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3) const {
        logImpl(QString(str).arg(t1).arg(t2).arg(t3));
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3, const T4 & t4) const {
        logImpl(QString(str).arg(t1).arg(t2).arg(t3).arg(t4));
        if(doHalt_) ::abort();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void operator()(const QString & str, const T1 &  t1, const T2 & t2, const T3 & t3, const T4 & t4, const T5 & t5) const {
        logImpl(QString(str).arg(t1).arg(t2).arg(t3).arg(t4).arg(t5));
        if(doHalt_) ::abort();
    }

private:
    void logImpl(const QString & message) const
    {
        qDebug() << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << level_ << " " << file_ << ":" << line_ << " "
                  << message;
    }

private:
    int     level_;
    bool    doHalt_;
    QString file_;
    int     line_;
};

class ScopedFunctionLogger
{
public:
    ScopedFunctionLogger(const QString & functionName) : functionName_(functionName)
    {
        LOG_DEBUG("--> %1" + functionName_);
    }

    ~ScopedFunctionLogger()
    {
        LOG_DEBUG("<-- %1" + functionName_);
    }

private:
    QString functionName_;
};
