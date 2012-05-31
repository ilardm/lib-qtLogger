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
    std::clog << __PRETTY_FUNCTION__ << std::endl;
#endif

    ll_string[ LL_EROR      ].sprintf( "ERROR" );
    ll_string[ LL_WARNING   ].sprintf( "WARN " );
    ll_string[ LL_LOG       ].sprintf( "LOG  " );
    ll_string[ LL_DEBUG     ].sprintf( "DEBUG" );

    ll_string[ LL_COUNT     ].sprintf( "     " );
}

QtLogger::~QtLogger()
{
#if ENABLE_LOGGER_LOGGING
    std::clog << __PRETTY_FUNCTION__ << std::endl;
#endif

    messageQueue.clear();
}

void QtLogger::foo( void* bar )
{
#if ENABLE_LOGGER_LOGGING
    std::clog << __PRETTY_FUNCTION__ << std::endl;
#endif
}

/**
 *
 */
void QtLogger::log(LOG_LEVEL level, QString message)
{
#if ENABLE_LOGGER_LOGGING
    std::clog << __PRETTY_FUNCTION__
            << " lvl: " << ll_string[ (level>LL_COUNT || level<0)?LL_COUNT:level ].toStdString()
            << "msg: \"" << message.toStdString() << "\""
            << std::endl;
#endif

    if ( level >= LL_COUNT ||
         level < 0
    ) {
#if ENABLE_LOGGER_LOGGING
        std::cerr << "incorrect log level" << std::endl;
#endif
        return;
    }

    if ( level > currentLevel )
    {
#if ENABLE_LOGGER_LOGGING
        std::clog << "log message rejected: currentLevel: "
                << ll_string[ currentLevel ].toStdString()
                << std::endl;
#endif
        return;
    }

    mqMutex.lock();
    messageQueue.enqueue( message );

#if ENABLE_LOGGER_LOGGING
    std::clog << "message queue size: "
            << messageQueue.size()
            << std::endl;
#endif
    mqMutex.unlock();

    return;
}
