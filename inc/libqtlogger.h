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
#include    <QMap>

/** main logger class.
 *
 * Implements sigletone pattern to use C macroses without constructing
 * object on each QtLogger#log call. Actually constructed on first call
 * any of macros defined below. Enqueues log messages and wakes up
 * processor thread which passes enqueued messages to writers
 * (#LogWriterInterface) registered via #ADD_LOG_WRITER macro. Before
 * application exit macro #FINISH_LOGGING must be called to correctly
 * process all passed log messages.
 *
 * Inherites QThread.
 *
 * @author Iya Arefiev
 */
class LIBQTLOGGER_EXPORT QtLogger
    : public QThread
{
public:

    /** available log levels enum.
     *
     * ordered by descending priority.
     */
    typedef enum {
        LL_ERROR,           /**< fatal error occured. always present in logs */
        LL_WARNING,         /**< warns about something */
        LL_WARNING_FINE,    /**< lower level warnings */
        LL_LOG,             /**< ordinary logs */
        LL_LOG_FINE,        /**< lower lovel logs */
        LL_DEBUG,           /**< debug logging */
        LL_DEBUG_FINE,      /**< more detailed logging */

        LL_STUB,            /**< not a log level, just a stub for non-level values */
        LL_COUNT            /**< count of elemens in #ll_string array */
    } LOG_LEVEL;

    typedef struct {
        LOG_LEVEL   level;
        bool        final;
    } MODULE_LEVEL;

public:
    static QtLogger& getInstance();
    ~QtLogger();

private:
    QtLogger();

public:
    void foo( void* );
    QString describeLogLevel( QtLogger::LOG_LEVEL );

    bool addWriter( LogWriterInterface* );
    QtLogger::LOG_LEVEL setModuleLevel( QString, LOG_LEVEL, bool=false );

    void log( LOG_LEVEL, QString, void*, size_t );

    void finishLogging();

protected:
    void run();
    QString hexData( const void*, const size_t );

protected:
    /** currently using log level threshold
     */
    LOG_LEVEL currentLevel;
    /** array of string representation of #LOG_LEVEL
     */
    QString ll_string[ LL_COUNT ];

    /** log messages queue
     */
    QQueue< QString > messageQueue;
    /** message queue guard
     */
    QMutex mqMutex;
    /** logger thread wait condition
     */
    QWaitCondition mqWait;
    /** logger thread exit condition
     */
    bool shutdown;

    /** list of registered log writers
     */
    QList< LogWriterInterface* > writersList;
    /** log writers list guard
     */
    QMutex wlMutex;

    QMap<QString, MODULE_LEVEL> moduleMap;
    QMutex mmMutex;
};

/** wrapper for QtLogger#addWriter.
 *
 * should be called before any logging appeared
 */
#define ADD_LOG_WRITER( writer )\
    QtLogger::getInstance().addWriter( writer )

/** wrapper for QtLogger#finishLogging.
 *
 * must be called before ending application execution
 */
#define FINISH_LOGGING()\
    QtLogger::getInstance().finishLogging()

/** separate filename from given path.
 *
 * searches position of '/' symbol in given
 * path and returns pointer to the next character.
 *
 * uses rindex function from strings.h
 */
#define FILENAME_FROM_PATH( path )\
    ( rindex(path,'/')?rindex(path,'/')+1:path )

#define SET_MODULE_LOGLEVEL( name, lvl )\
    QtLogger::LOG_LEVEL __loglevelFor##name = QtLogger::getInstance().setModuleLevel( QString( FILENAME_FROM_PATH(__FILE__) ), lvl )

/** wrapper for QtLogger#log.
 *
 * creates log message in following format:<br>
 * {current_time} {log_level-string} {filename}:{line_number} [{thread_id}] {function_signature} {passed_format}
 *
 * @param lvl       #LOG_LEVEL
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
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

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_ERROR,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_ERROR(fmt, args...)\
    LOG_WRITE( QtLogger::LL_ERROR, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_ERROR
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_ERRORX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_ERROR, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_WARN(fmt, args...)\
    LOG_WRITE( QtLogger::LL_WARNING, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_WARNX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_WARNING, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING_FINE,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_WARNF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_WARNING_FINE, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_WARNXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_WARNING_FINE, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_LOG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_LOG, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_LOGX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_LOG, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG_FINE,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_LOGF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_LOG_FINE, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_LOGXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_LOG_FINE, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_DEBUG(fmt, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_DEBUGX(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG, fmt, data, datasz , ##args )

/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG_FINE,
 * data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_DEBUGF(fmt, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG_FINE, fmt, NULL, 0 , ##args )
/** wrapper for #LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_DEBUGXF(fmt, data, datasz, args...)\
    LOG_WRITE( QtLogger::LL_DEBUG_FINE, fmt, data, datasz , ##args )
