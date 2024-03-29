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

#include    <iostream>

#include    <QMapIterator>
#include    <QStringList>

#include    "libqtlogger_common.h"
#include    "libqtlogger.h"

using namespace ilardm::lib::qtlogger;

/** logger object constructor.
 *
 * initializes internal QtLogger#currentLevel,
 * initializes #ll_string array with string represenattion of #LOG_LEVEL,
 * initializes QtLogger#defaultModuleLevel with "-default",
 * initializes QtLogger#settingsSection with "logging",
 * launches logger thread QtLogger#run
 */
QtLogger::QtLogger()
    : defaultModuleLevel( "-default" ),
      currentLevel( LL_WARNING ),
      mmMutex(QMutex::Recursive),    // allow loadModuleLevels to lock
      settings( NULL ),
      settingsSection( "logging" )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    // order matters: must be the same as in LOG_LEVEL enum!
    ll_string << "ERROR "       // LL_ERROR
              << "WARN  "       // LL_WARNING
              << "WARN+ "       // LL_WARNING_FINE
              << "log   "       // LL_LOG
              << "log+  "       // LL_LOG_FINE
              << "debug "       // LL_DEBUG
              << "debug+"       // LL_DEBUG_FINE
              << "      "       // LL_STUB
              ;

    this->start();
}

/** logger object destructor.
 *
 * currently does nothing.
 */
QtLogger::~QtLogger()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif
}

/** singleton logger object constructor.
 *
 * constructs static QtLogger object and returns
 * its reference
 *
 * @return reference to static QtLogger object
 */
QtLogger& QtLogger::getInstance()
{
    static QtLogger _instance;

    return _instance;
}

/** dummy function.
 *
 * does nothing.
 */
void QtLogger::foo( void* bar )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif
}

/** decide whether to use file name or class/function as module name.
 *
 * uses filename or
 * namespace or
 * namespace + class
 * as module name.
 *
 * if called from top-level (i.e. outside #main) --
 * uses file name as module name.
 *
 * if called from #main -- module name = "main".
 *
 * if called from fooNameSpace::main() --
 * module name = "fooNameSpace".
 *
 * if called from fooNameSpace::FooClass::fooMethod() --
 * module name = "fooNameSpace--FooClass".
 *
 * @param funcname  should be #FUNCTION_NAME
 * @param filename  should be __FILE__
 *
 * @return module name
 */
QString QtLogger::determineModule( const char* funcname, const char* filename )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " funcname: \""
            << ( funcname?funcname:"(null)" ) << "\""
            << " filename: \""
            << ( filename?filename:"(null)" ) << "\""
            << std::endl;
#endif

    // TODO: optimize
    static const char* defaultModule = (const char*)(QtLogger::getInstance().defaultModuleLevel.toStdString().c_str());

    QString ret( funcname ? funcname : defaultModule );
    if ( ret.contains("(") )
    {
        // function

        int delim = ret.indexOf("(");
        int space = ret.lastIndexOf( " ", delim );

        ret = ret.mid( space, delim - space );

        if ( ret.contains( "::" ) )
        {
            // class || namespace

            delim = ret.lastIndexOf( "::" );
            ret = ret.left( delim );
        }
    }
    else
    {
        // file

        ret = QString( filename ? LQTL_FILENAME_FROM_PATH( filename ) : defaultModule );
    }

    ret = ret.replace(":", "-").trimmed();

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " module: \""
            << ret.toStdString() << "\""
            << std::endl;
#endif

    return ret;
}

/** converts #LOG_LEVEL value to string representation.
 *
 * uses initialized in QtLogger#QtLogger #ll_string array.
 *
 * @param level log level to convert
 *
 * @return string representation of passed level<br>
 *         or stub if passed level is out of bounds
 */
QString QtLogger::describeLogLevel(QtLogger::LOG_LEVEL level)
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " level: "
            << level
            << " ("
            << ll_string[((level>=0 && level<=LL_STUB)?level:LL_STUB)].toStdString()
            << ")"
            << std::endl;
#endif

    return ll_string[ ((level>=0 && level<=LL_STUB) ? level : LL_STUB) ];
}

/** logger thread.
 *
 * runs in infinite loop until QtLogger#shutdown flag is set,
 * waites on QtLogger#mqWait condition until message added into
 * QtLogger#messageQueue,
 * passes all messages in queue to registered log writes from
 * QtLogger#writersList
 * and waits again.
 *
 * since messages may be enqueued while log writers doing some stuff,
 * this method executes log writers until queue is empty and only then
 * goes to sleep on condition.
 *
 * twice locking message queue: right after wake up and before going to sleep
 * to ensure if queue is empty.
 *
 * writers list is locked until all writers with current message is executed.
 */
void QtLogger::run()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " start"
            << std::endl;
#endif

    QString message;

    while ( !shutdown )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " waiting"
                << std::endl;
#endif
        mqMutex.lock();
        mqWait.wait( &mqMutex );
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " woken"
                << std::endl;
#endif
        if ( !messageQueue.isEmpty() )
        {
            message = messageQueue.dequeue();
        }
#if LQTL_ENABLE_LOGGER_LOGGING
        else
        {
            std::clog << FUNCTION_NAME
                    << " queue already empty"
                    << std::endl;
        }
#endif
        mqMutex.unlock();

        while ( !message.isEmpty() )
        {
#if LQTL_ENABLE_LOGGER_LOGGING
            std::clog << FUNCTION_NAME
                    << " pass message \""
                    << message.toStdString()
                    << "\" to writers"
                    << std::endl;
#endif

            wlMutex.lock();
            if ( !writersList.isEmpty() )
            {
                QListIterator<LogWriterInterface*> iter( writersList );
                while ( iter.hasNext() )
                {
                    LogWriterInterface* writer = iter.next();
#if LQTL_ENABLE_LOGGER_LOGGING
                    bool status =
#endif
                    writer->writeLog( message );

#if LQTL_ENABLE_LOGGER_LOGGING
                    std::clog << FUNCTION_NAME
                            << QString().sprintf( " writer @ %p returned %c",
                                                  writer,
                                                  (status?'t':'F')
                                                ).toStdString()
                            << std::endl;
#endif
                }
            }
            wlMutex.unlock();

            mqMutex.lock();
            if ( !messageQueue.isEmpty() )
            {
                message = messageQueue.dequeue();
            }
            else
            {
                message.clear();
            }
            mqMutex.unlock();
        }
    }

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " end"
            << std::endl;
#endif

    this->quit();
}

/** converts passed data to hex representation.
 *
 * uses formatting like hexdump(1) utility
 *
 * @param data      data buffer to convert
 * @param datasz    number of bytes to convert
 *
 * @return string representation of passed data<br>
 *         or blank string if datasize equals 0
 *         or data pointer equals NULL
 */
QString QtLogger::hexData( const void* data, const size_t datasz )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " data @"
            << QString().sprintf( " %p", data ).toStdString()
            << " size: "
            << datasz
            << std::endl;
#endif

    QString result("\n");
    QString hexline("");
    QString asciidata("");
    quint8* p = (quint8*)data;

    // typical line
    // 0x0010: 0000 0000 0000 0000 0000 0000 032a 0010 '.............*..'
    // length of hexes should be 47

    for ( size_t i = 0; i < datasz; i++ )
    {
        if ( i % 16 == 0 )
        {
            hexline.append( QString().sprintf( "0x%04X: ", (uint)i ) );
        }

        if ( i % 2 == 0 )
        {
            hexline.append( QString().sprintf( "%02x", p[i] ) );
        }
        else
        {
            hexline.append( QString().sprintf( "%02x ", p[i] ) );
        }
        // append with char (if printable) or with '.' symbol
        asciidata.append( (const char)( (p[i] >= 0x20 && p[i] <= 0x7e) ? p[i] : 0x2e ) );

        if ( (i+1) % 16 == 0)
        {
            result.append( QString().sprintf( "%-50s'%s'\n",
                                              hexline.toStdString().c_str(),
                                              asciidata.toStdString().c_str()
                                              )
                           );

            asciidata.clear();
            hexline.clear();
        }
    }

    if ( !asciidata.isEmpty() )
    {
        result.append( QString().sprintf( "%-50s'%s'\n",
                                          hexline.toStdString().c_str(),
                                          asciidata.toStdString().c_str()
                                          )
                       );
    }

    return result;
}

/** registers one more log writer object.
 *
 * checks passed pointer,
 * locks QtLogger#wlMutex,
 * appends pointer to QtLogger#writersList
 * and unlocks mutex
 *
 * @param writer
 *
 * @return true if successfully added<br>
 *         false otherwise (i.e. pointer is NULL)
 */
bool QtLogger::addWriter( LogWriterInterface* writer )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " writer: "
            << (writer?(QString().sprintf( "%p", writer ).toStdString()):"(null)")
            << std::endl;
#endif

    if ( !writer )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::cerr << FUNCTION_NAME
                << " NULL writer"
                << std::endl;
#endif
        return false;
    }

   wlMutex.lock();
   writersList.append( writer );

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " wl.size: "
            << writersList.size()
            << std::endl;
#endif
    wlMutex.unlock();

    return true;
}

/** set log level for module.
 *
 * assigns log level for given module.
 * if log level already set - reassigns it if
 * not marked as final or final param is set to true
 *
 * @param module    module name
 * @param lvl       log level for module
 * @param final     determines whether override is forced
 *
 * @return assigned log level for module
 */
QtLogger::LOG_LEVEL QtLogger::setModuleLevel( QString module, LOG_LEVEL lvl, bool final )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " module: "
            << module.toStdString()
            << " level: "
            << describeLogLevel( lvl ).toStdString()
            << " final: "
            << ( final?"T":"f" )
            << std::endl;
#endif

    if ( lvl < 0
         || lvl >= LL_STUB
    ) {
        lvl = currentLevel;

#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " incorrect log level passed. set to default"
                << std::endl;
#endif
    }

    bool insert = false;

    // check if exists
    const MODULE_LEVEL* mlvl = getModuleLevel( module );
    if ( !mlvl )
    {
        insert = true;
    }
    else
    {
        // reset if !final
        if ( mlvl->final
             && !final
        ) {
#if LQTL_ENABLE_LOGGER_LOGGING
            std::clog << FUNCTION_NAME
                    << " log level for this module already final. rejected"
                    << std::endl;
#endif
            return mlvl->level;
        }
        else
        {
#if LQTL_ENABLE_LOGGER_LOGGING
            std::clog << FUNCTION_NAME
                    << " replace existsing log level"
                    << std::endl;
#endif
            delete( mlvl );
            insert = true;
        }
    }

    if ( insert )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " insert new loglevel for module"
                << std::endl;
#endif
        MODULE_LEVEL* nmlvl = new MODULE_LEVEL();
        nmlvl->level = lvl;
        nmlvl->final = final;

        if ( !nmlvl )
        {
#if LQTL_ENABLE_LOGGER_LOGGING
            std::cerr << FUNCTION_NAME
                    << " unable to create loglevel for module"
                    << std::endl;
#endif
            return LL_STUB;
        }

        mmMutex.lock();
        moduleMap.insert( module, nmlvl );
        mmMutex.unlock();

        return lvl;
    }

    return LL_STUB;
}

/** retrieve log level for given module.
 *
 * @param module module name
 *
 * @return mdule log level structure if log level already assigned to module<br>
 *         NULL if no module found.
 */
const QtLogger::MODULE_LEVEL* QtLogger::getModuleLevel( QString module )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " module: "
            << module.toStdString()
            << std::endl;
#endif

    const MODULE_LEVEL* ret;

    mmMutex.lock();
    ret = moduleMap.value( module, NULL );
    mmMutex.unlock();

    return ret;
}

/** set main-application settings object.
 *
 * should be set before QtLogger#saveModuleLevels
 * or Qtlogger#loadModuleLevels invoked
 *
 * @param settings main-application settings object
 *
 * @return true if settings object not NULL<br>
 *         false otherwise
 */
bool QtLogger::setSettingsObject( QSettings* settings )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " settings file \""
            << ( settings?settings->fileName().toStdString().c_str():"(null)" ) << "\""
            << std::endl;
#endif

    if ( settings )
    {
        this->settings = settings;
        return true;
    }

    return false;
}

/** saves log levels for modules.
 *
 * saves also default log level with module name
 * as set in QtLogger#defaultModuleLevel.
 *
 * @return true if successfully saved<br>
 *         false otherwise
 */
bool QtLogger::saveModuleLevels()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    if ( settings )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " using settings object"
                << std::endl;
#endif
        settings->beginGroup( settingsSection );
        settings->setValue( defaultModuleLevel, (int)(currentLevel) );
        mmMutex.lock();

        QMapIterator< QString, MODULE_LEVEL* > iter( moduleMap );
        while ( iter.hasNext() )
        {
            iter.next();

#if LQTL_ENABLE_LOGGER_LOGGING
            std::clog << FUNCTION_NAME
                    << " module: \""
                    << iter.key().toStdString() << "\""
                    << " level: "
                    << (int)(iter.value()->level)
                    << std::endl;
#endif

            settings->setValue( iter.key(), (int)(iter.value()->level) );
        }
        mmMutex.unlock();
        settings->endGroup();
        settings->sync();

        return true;
    }

#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " no settings object. log levels would not be saved"
                << std::endl;
#endif

    return false;
}

/** loads log levels for modules from config.
 *
 * if QtLogger#defaultModuleLevel found in config file
 * resets QtLogger#currentLevel to its value
 *
 * @return true if successfully loaded<br>
 *         false otherwise
 */
bool QtLogger::loadModuleLevels()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    if ( settings )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " using settings object"
                << std::endl;
#endif
        settings->beginGroup( settingsSection );
        QStringList keys = settings->allKeys();
        mmMutex.lock();

        foreach ( const QString &key, keys )
        {
            setModuleLevel( key,
                            (LOG_LEVEL)(settings->value( key, currentLevel ).toInt()),
                            true
                          );

            if ( key == defaultModuleLevel )
            {
                currentLevel = (LOG_LEVEL)settings->value( key, currentLevel ).toInt();

#if LQTL_ENABLE_LOGGER_LOGGING
                std::clog << FUNCTION_NAME
                        << " restored default log level: "
                        << LQTL_QSTRINGCHAR( describeLogLevel( currentLevel ) )
                        << std::endl;
#endif
            }
        }

        mmMutex.unlock();
        settings->endGroup();
        settings->sync();

        return true;
    }

#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " no settings object. log levels would not be loaded"
                << std::endl;
#endif
    return false;
}

/** retrieve log levels description
 *
 * @return #ll_string
 */
QStringList QtLogger::getLogLevelsDescription()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    return ll_string;
}

/** retrieve current log levels for modules map
 *
 * @return #moduleMap
 */
QMap< QString, QtLogger::MODULE_LEVEL* > QtLogger::getModulesMap()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    return moduleMap;
}

/** log passed message.
 *
 * checks if passed log message level is lesser than
 * assigned for module,
 * if no loglevel for module record found - creates
 * one with QtLogger#currentLevel and checks if
 * log messae level is lesser than
 * QtLogger#currentLevel,
 * converts passed data into hex string (if any) and
 * appends it to log message,
 * enqueues passed log message into QtLogger#messageQueue
 * and wakesup QtLogger#run thread.
 *
 * @param level     message log level
 * @param module    module name
 * @param message   formed log message
 * @param data      data to dump in hex if any
 * @param datasz    size of data to dump
 */
void QtLogger::log(LOG_LEVEL level, QString module, QString message, const void* data, size_t datasz)
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " lvl: " << ll_string[ (level>=LL_STUB || level<0)?LL_STUB:level ].toStdString()
            << " module: \"" << module.toStdString() << "\""
            << " msg: \"" << message.toStdString() << "\""
            << std::endl;
#endif

    if ( level >= LL_STUB ||
         level < 0
    ) {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::cerr << FUNCTION_NAME
                << "incorrect log level"
                << std::endl;
#endif
        return;
    }

    bool checkCurrentLogLevel = true;

    const MODULE_LEVEL* mlvl = getModuleLevel( module );
    if ( mlvl
         && level > mlvl->level
    ) {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " log message rejected: moduleLevel: "
                << ll_string[ mlvl->level ].toStdString()
                << std::endl;
#endif
        return;
    }
    else if ( !mlvl )
    {
        // set default log level for unknown module
        setModuleLevel( module, currentLevel );
    }
    else
    {
        checkCurrentLogLevel = false;
    }


    if ( checkCurrentLogLevel
         && level > currentLevel )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " log message rejected: currentLevel: "
                << ll_string[ currentLevel ].toStdString()
                << std::endl;
#endif
        return;
    }

    if ( data
         && datasz > 0
    ) {
        message.append( hexData( data, datasz ) );
    }

    mqMutex.lock();
    messageQueue.enqueue( message );

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " message queue size: "
            << messageQueue.size()
            << std::endl;
#endif

    mqWait.wakeAll();
    mqMutex.unlock();

    return;
}

/** finish logging.
 *
 * set QtLogger#shutdown flag,
 * wake up QtLogger#run thread,
 * waits until it exits,
 * deletes objects in QtLogger#writersList
 * and cleans QtLogger#writersList list,
 * saves og levels for modules
 * and deletes items in QtLogger#moduleMap
 */
void QtLogger::finishLogging()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    mqMutex.lock();
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " messageQueue size: "
            << messageQueue.size()
            << std::endl;
#endif
    shutdown = true;

    mqWait.wakeAll();
    mqMutex.unlock();

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " wait thread to end"
            << std::endl;
#endif
    this->wait();

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " cleanup writers list"
            << std::endl;
#endif
    while ( !writersList.isEmpty() )
    {
        delete( writersList.front() );
        writersList.pop_front();
    }

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " saving logger config"
            << std::endl;
#endif
    saveModuleLevels();

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " cleanup moduleMap"
            << std::endl;
#endif
    QMapIterator< QString, MODULE_LEVEL* > iter( moduleMap );
    while ( iter.hasNext() )
    {
        delete( iter.next().value() );
    }

#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " done"
            << std::endl;
#endif
}
