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
        LL_LOG,
        LL_DEBUG,

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
    void log( LOG_LEVEL, QString );

protected:
    void run();

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

#define FILENAME_FROM_PATH( path )\
    ( rindex(path,'/')?rindex(path,'/')+1:path )

#define LOG_WRITE(lvl, fmt, args... )\
    QtLogger::getInstance().log( lvl,\
                                 QString().sprintf( "%s %s %s:%d [%p] %s " fmt,\
                                                    QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),\
                                                    QtLogger::getInstance().describeLogLevel(lvl).toStdString().c_str(),\
                                                    FILENAME_FROM_PATH(__FILE__),\
                                                    __LINE__,\
                                                    (void*)QThread::currentThreadId(),\
                                                    FUNCTION_NAME ,\
                                                    ##args\
                                                  )\
                               )

#define LOG_ERROR(fmt, args...)\
    LOG_WRITE( QtLogger::LL_ERROR, fmt , ##args )

#define LOG_WARN(fmt, args...)\
    LOG_WRITE( QtLogger::LL_WARNING, fmt , ##args )

#define LOG_LOG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_LOG, fmt , ##args )

#define LOG_DEBUG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG, fmt , ##args )
