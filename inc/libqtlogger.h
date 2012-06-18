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

#include    <string.h>

#include    "libqtlogger_common.h"
#include    "logwriterinterface.h"

#include    <QString>
#include    <QQueue>
#include    <QMutex>
#include    <QWaitCondition>
#include    <QThread>
#include    <QDateTime>
#include    <QMap>
#include    <QTextStream>
#include    <QFile>
#include    <QTextStream>
#include    <QSettings>

namespace ilardm {
namespace lib {
namespace qtlogger {

/** main logger class.
 *
 * Implements sigletone pattern to use C macroses without constructing
 * object on each QtLogger#log call. Actually constructed on first call
 * any of macros defined below. Enqueues log messages and wakes up
 * processor thread which passes enqueued messages to writers
 * (#LogWriterInterface) registered via #LQTL_ADD_LOG_WRITER macro. Before
 * application exit macro #LQTL_FINISH_LOGGING must be called to correctly
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

    /** auxiliary structure to hold log level for module.
     */
    typedef struct {
        LOG_LEVEL   level;  /**< log level for module*/
        bool        final;  /**< determines whether log level may be owerridden */
    } MODULE_LEVEL;

public:
    static QtLogger& getInstance();
    ~QtLogger();

private:
    QtLogger();

public:
    void foo( void* );
    static QString determineModule( const char*, const char* );
    QString describeLogLevel( QtLogger::LOG_LEVEL );

    bool addWriter( LogWriterInterface* );

    LOG_LEVEL setModuleLevel( QString, LOG_LEVEL, bool=false );
    const MODULE_LEVEL* getModuleLevel( QString );
    bool setSettingsObject( QSettings* = NULL );
    bool saveModuleLevels();
    bool loadModuleLevels();

    void log( LOG_LEVEL, QString, QString, const void*, size_t );

    void finishLogging();

protected:
    void run();
    QString hexData( const void*, const size_t );

protected:
    /** represents default log level <i>module name</i> in config file
      */
    QString defaultModuleLevel;
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

    /** mapping of #LOG_LEVEL to module name
     */
    QMap< QString, MODULE_LEVEL* > moduleMap;
    /** #moduleMap guard
     */
    QMutex mmMutex;

    /** main application settings object where logger settings would be stored
     */
    QSettings* settings;
    /** logger settings section
     */
    QString settingsSection;
};

/** wrapper for QtLogger#addWriter.
 *
 * should be called before any logging appeared
 */
#define LQTL_ADD_LOG_WRITER( writer )\
    ilardm::lib::qtlogger::QtLogger::getInstance().addWriter( writer )

/** wrapper for QtLogger#setSettingsObject
 */
#define LQTL_SET_SETTINGS_OBJECT( settings )\
    ilardm::lib::qtlogger::QtLogger::getInstance().setSettingsObject( settings )

/** wrapper for QtLogger#saveModuleLevels.
 */
#define LQTL_SAVE_LOG_CONFIG()\
    ilardm::lib::qtlogger::QtLogger::getInstance().saveModuleLevels()

/** wrapper for QtLogger#loadModuleLevels.
 */
#define LQTL_LOAD_LOG_CONFIG()\
    ilardm::lib::qtlogger::QtLogger::getInstance().loadModuleLevels()

/** loads logger config.
 *
 * should be called first (right after QSettings object created)
 * to load all log levels for modules.
 *
 * defines boolean __qtLoggerConfigFileSet set to
 * true if settings object was set && settings was loaded
 * ( by calling QtLogger#setSettingsObject combining
 * with QtLogger#loadModuleLevels by '&&', so no
 * settings loading if QtLogger#setSettingsObject
 * returned false ).
 *
 * @param config config file name
 */
#define LQTL_START_LOGGING( config )\
    bool __qtLoggerConfigFileSet = ( ilardm::lib::qtlogger::QtLogger::getInstance().setSettingsObject( config ) &&\
                                     ilardm::lib::qtlogger::QtLogger::getInstance().loadModuleLevels() )

/** wrapper for QtLogger#finishLogging.
 *
 * must be called before ending application execution
 */
#define LQTL_FINISH_LOGGING()\
    ilardm::lib::qtlogger::QtLogger::getInstance().finishLogging()

/** separate filename from given path.
 *
 * searches position of '/' symbol in given
 * path and returns pointer to the next character.
 *
 * uses rindex function from strings.h
 */
#if defined ( Q_OS_LINUX )
#define LQTL_FILENAME_FROM_PATH( path )\
    ( strrchr(path,'/')?strrchr(path,'/')+1:path )
#elif defined ( Q_OS_WIN32 )
#define LQTL_FILENAME_FROM_PATH( path )\
    ( strrchr(path,'\\')?strrchr(path,'\\')+1:path )
#else
#error "unsupported platform"
#endif

/** auxiliary macro to suppress compiler warnings.
 */
#define LQTL_UNUSED_VARIABLE(var)\
    ((void)var)

/** auxiliary macro to convert QString to const char*.
  */
#define LQTL_QSTRINGCHAR( str )\
    str.toLocal8Bit().constData()

/** wrapper for QtLogger#determineModule.
 */
#define LQTL_DETERMINE_MODULE()\
    ilardm::lib::qtlogger::QtLogger::determineModule(FUNCTION_NAME, __FILE__)

/** wrapper for QtLogger#setModuleLevel.
 *
 * defines QtLogger#LOG_LEVEL __logLevelFor{...}
 * variable. {...} substituded with passed name argument.
 *
 * @param name  name for defined variable
 * @param lvl   log level for current module
 */
#define LQTL_SET_MODULE_LOGLEVEL( name, lvl )\
    ilardm::lib::qtlogger::QtLogger::LOG_LEVEL __loglevelFor##name = ilardm::lib::qtlogger::QtLogger::getInstance().setModuleLevel( LQTL_DETERMINE_MODULE, lvl )

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
#define LQTL_LOG_WRITE(lvl, fmt, data, datasz, ... )\
    ilardm::lib::qtlogger::QtLogger::getInstance().log( lvl,\
                                 LQTL_DETERMINE_MODULE(),\
                                 QString().sprintf( "%s %s %16s:%-5d\t[%p] %s " fmt,\
                                                    QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),\
                                                    ilardm::lib::qtlogger::QtLogger::getInstance().describeLogLevel(lvl).toStdString().c_str(),\
                                                    LQTL_FILENAME_FROM_PATH(__FILE__),\
                                                    __LINE__,\
                                                    (void*)QThread::currentThreadId(),\
                                                    FUNCTION_NAME ,\
                                                    ##__VA_ARGS__\
                                                  ),\
                                 data, datasz\
                               )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_ERROR
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_ERRORX(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_ERROR,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_ERRORX
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_ERROR(fmt, ...)\
    LOG_ERRORX( fmt, NULL, 0 , ##args )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_WARNX(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_WARNING,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_WARNX
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_WARN(fmt, ...)\
    LOG_WARNX( fmt, NULL, 0 , ##__VA_ARGS__ )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_WARNING_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_WARNXF(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_WARNING_FINE,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_WARNXF
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_WARNF(fmt, ...)\
    LOG_WARNXF( fmt, NULL, 0 , ##__VA_ARGS__ )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_LOGX(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_LOG,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_LOGX
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_LOG(fmt, ...)\
    LOG_LOGX( fmt, NULL, 0 , ##__VA_ARGS__ )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_LOG_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_LOGXF(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_LOG_FINE,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_LOGXF
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
#define LOG_LOGF(fmt, ...)\
    LOG_LOGXF( fmt, NULL, 0 , ##__VA_ARGS__ )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_DEBUGX(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_DEBUG,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_DEBUGX
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_DEBUG(fmt, ...)\
    LOG_DEBUGX( fmt, NULL, 0 , ##__VA_ARGS__ )

/** wrapper for #LQTL_LOG_WRITE
 *
 * substitudes lvl with QtLogger#LL_DEBUG_FINE
 *
 * @param fmt       message format
 * @param data      pointer to data buffer dumped in hex
 * @param datasz    size of data buffer
 * @param args      arguments for fmt
 */
#define LOG_DEBUGXF(fmt, data, datasz, ...)\
    LQTL_LOG_WRITE( ilardm::lib::qtlogger::QtLogger::LL_DEBUG_FINE,\
                    fmt, data, datasz , ##__VA_ARGS__ )
/** wrapper for #LOG_DEBUGXF
 *
 * substitudes data with NULL,
 * datasz with 0
 *
 * allows invoking without arguments
 *
 * @param fmt       message format
 * @param args      arguments for fmt
 */
#define LOG_DEBUGF(fmt, ...)\
    LOG_DEBUGXF( fmt, NULL, 0 , ##__VA_ARGS__ )

}   // qtlogger
}   // lib
}   // ilardm
