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

#include    "libqtlogger_common.h"
#include    "libqtlogger.h"

QtLogger::QtLogger()
    : currentLevel( LL_DEBUG )
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    ll_string[ LL_ERROR     ].sprintf( "ERROR" );
    ll_string[ LL_WARNING   ].sprintf( "WARN " );
    ll_string[ LL_LOG       ].sprintf( "LOG  " );
    ll_string[ LL_DEBUG     ].sprintf( "DEBUG" );

    ll_string[ LL_STUB      ].sprintf( "     " );

    messageQueue.enqueue(
            QString("logger startup: %1").arg(
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                )
            );

    this->start();
}

QtLogger::~QtLogger()
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif
}

QtLogger& QtLogger::getInstance()
{
    static QtLogger _instance;

    return _instance;
}

void QtLogger::foo( void* bar )
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif
}

QString QtLogger::describeLogLevel(QtLogger::LOG_LEVEL level)
{
#if ENABLE_LOGGER_LOGGING
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

/**
 *
 */
void QtLogger::run()
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " start"
            << std::endl;
#endif

    QString message;

    while ( !shutdown )
    {
#if ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " waiting"
                << std::endl;
#endif
        mqMutex.lock();
        mqWait.wait( &mqMutex );
#if ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " woken"
                << std::endl;
#endif
        if ( !messageQueue.isEmpty() )
        {
            message = messageQueue.dequeue();
        }
#if ENABLE_LOGGER_LOGGING
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
#if ENABLE_LOGGER_LOGGING
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
#if ENABLE_LOGGER_LOGGING
                    bool status =
#endif
                    writer->writeLog( message );

#if ENABLE_LOGGER_LOGGING
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

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " end"
            << std::endl;
#endif

    this->quit();
}

QString QtLogger::hexData( const void* data, const size_t datasz )
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " data @"
            << QString().sprintf( " %p", data ).toStdString()
            << " size: "
            << datasz
            << std::endl;
#endif

    QString result("\n");
    quint8* p = (quint8*)data;

    for ( size_t i = 0; i < datasz; i++ )
    {
        if ( i % 16 == 0 )
        {
            result.append( QString().sprintf( "0x%04X: ", (uint)i ) );
        }

        if ( i % 2 == 0 )
        {
            result.append( QString().sprintf( "%02x", p[i] ) );
        }
        else
        {
            result.append( QString().sprintf( "%02x ", p[i] ) );
        }

        if ( (i+1) % 16 == 0)
        {
            result.replace( result.length()-1, 1, "\n" );
        }
    }

    return result;
}

bool QtLogger::addWriter( LogWriterInterface* writer )
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " writer: "
            << (writer?(QString().sprintf( "%p", writer ).toStdString()):"(null)")
            << std::endl;
#endif

    if ( !writer )
    {
#if ENABLE_LOGGER_LOGGING
        std::cerr << FUNCTION_NAME
                << " NULL writer"
                << std::endl;
#endif
        return false;
    }

   wlMutex.lock();
   writersList.append( writer );

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " wl.size: "
            << writersList.size()
            << std::endl;
#endif
    wlMutex.unlock();

    return true;
}

/**
 *
 */
void QtLogger::log(LOG_LEVEL level, QString message, void* data, size_t datasz)
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " lvl: " << ll_string[ (level>=LL_STUB || level<0)?LL_STUB:level ].toStdString()
            << " msg: \"" << message.toStdString() << "\""
            << std::endl;
#endif

    if ( level >= LL_STUB ||
         level < 0
    ) {
#if ENABLE_LOGGER_LOGGING
        std::cerr << FUNCTION_NAME
                << "incorrect log level"
                << std::endl;
#endif
        return;
    }

    if ( level > currentLevel )
    {
#if ENABLE_LOGGER_LOGGING
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

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " message queue size: "
            << messageQueue.size()
            << std::endl;
#endif

    mqWait.wakeAll();
    mqMutex.unlock();

    return;
}

void QtLogger::finishLogging()
{
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    mqMutex.lock();
#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " messageQueue size: "
            << messageQueue.size()
            << std::endl;
#endif
    shutdown = true;

    mqWait.wakeAll();
    mqMutex.unlock();

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " wait thread to end"
            << std::endl;
#endif
    this->wait();

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " cleanup writers list"
            << std::endl;
#endif
    while ( !writersList.isEmpty() )
    {
        delete( writersList.front() );
        writersList.pop_front();
    }

#if ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " done"
            << std::endl;
#endif
}
