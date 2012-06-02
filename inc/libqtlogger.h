// Copyright (c) 2012, Ilya Arefiev <arefiev.id@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//  * Neither the name of the author nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include    <strings.h>

#include    "libqtlogger_common.h"
#include    "logwriterinterface.h"

#include    <QString>
#include    <QQueue>
#include    <QMutex>
#include    <QWaitCondition>
#include    <QThread>
#include    <QDateTime>

class LIBQTLOGGER_EXPORT QtLogger
    : public QThread
{
public:
    typedef enum {
        LL_ERROR,
        LL_WARNING,
        LL_WARNING_FINE,
        LL_LOG,
        LL_LOG_FINE,
        LL_DEBUG,
        LL_DEBUG_FINE,

        LL_STUB,
        LL_COUNT
    } LOG_LEVEL;

public:
    static QtLogger& getInstance();
    ~QtLogger();

private:
    QtLogger();

public:
    void foo( void* );
    QString describeLogLevel( QtLogger::LOG_LEVEL );

    bool addWriter( LogWriterInterface* );
    void log( LOG_LEVEL, QString, void*, size_t );

    void finishLogging();

protected:
    void run();
    QString hexData( const void*, const size_t );

protected:
    LOG_LEVEL currentLevel;
    QString ll_string[ LL_COUNT ];

    QQueue< QString > messageQueue;
    QMutex mqMutex;
    QWaitCondition mqWait;
    bool shutdown;

    QList< LogWriterInterface* > writersList;
    QMutex wlMutex;
};

#define ADD_LOG_WRITER( writer )\
    QtLogger::getInstance().addWriter( writer )

#define FINISH_LOGGING()\
    QtLogger::getInstance().finishLogging()

#define FILENAME_FROM_PATH( path )\
    ( rindex(path,'/')?rindex(path,'/')+1:path )

#define LOG_WRITE(lvl, fmt, data, datasz, args... )\
    QtLogger::getInstance().log( lvl,\
                                 QString().sprintf( "%s %s %16s:%d\t[%p] %s " fmt,\
                                                    QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),\
                                                    QtLogger::getInstance().describeLogLevel(lvl).toStdString().c_str(),\
                                                    FILENAME_FROM_PATH(__FILE__),\
                                                    __LINE__,\
                                                    (void*)QThread::currentThreadId(),\
                                                    FUNCTION_NAME ,\
                                                    ##args\
                                                  ),\
                                 data, datasz\
                               )

#define LOG_ERROR(fmt, args...)\
    LOG_WRITE( QtLogger::LL_ERROR, fmt, NULL, 0 , ##args )
#define LOG_ERRORX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_ERROR, fmt, data, datasz , ##args )

#define LOG_WARN(fmt, args...)\
    LOG_WRITE( QtLogger::LL_WARNING, fmt, NULL, 0 , ##args )
#define LOG_WARNX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_WARNING, fmt, data, datasz , ##args )

#define LOG_WARNF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_WARNING_FINE, fmt, NULL, 0 , ##args )
#define LOG_WARNXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_WARNING_FINE, fmt, data, datasz , ##args )

#define LOG_LOG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_LOG, fmt, NULL, 0 , ##args )
#define LOG_LOGX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_LOG, fmt, data, datasz , ##args )

#define LOG_LOGF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_LOG_FINE, fmt, NULL, 0 , ##args )
#define LOG_LOGXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_LOG_FINE, fmt, data, datasz , ##args )

#define LOG_DEBUG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG, fmt, NULL, 0 , ##args )
#define LOG_DEBUGX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG, fmt, data, datasz , ##args )

#define LOG_DEBUGF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG_FINE, fmt, NULL, 0 , ##args )
#define LOG_DEBUGXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG_FINE, fmt, data, datasz , ##args )
