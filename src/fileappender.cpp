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

#include    <QDateTime>

#include    "libqtlogger_common.h"
#include    "fileappender.h"

using namespace ilardm::lib::qtlogger;

/** log file and stream constructor.
 *
 * constructs file handle,
 * stream from file
 * and set FileAppender#valid flag if no
 * error occured
 */
FileAppender::FileAppender( QString filename )
    : LogWriterInterface(),
      logfile( filename ),
      lfStream( &logfile ),
      valid( false )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME
            << " filename: "
            << filename.toStdString()
            << std::endl;
#endif

    bool status = logfile.open( QIODevice::Append
                                | QIODevice::Text
                              );

    if ( status )
    {
        if ( lfStream.status() == QTextStream::Ok )
        {
            valid = true;
        }
        else
        {
#if LQTL_ENABLE_LOGGER_LOGGING
            std::cerr << FUNCTION_NAME
                    << " unable to create text stream: "
                    << lfStream.status()
                    << std::endl;
#endif
            return;
        }
    }
    else
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::cerr << FUNCTION_NAME
                << " unable to open file: "
                << logfile.error()
                << std::endl;
#endif
        return;
    }

    lfStream << QString("=======================================\n");
    lfStream << QString("logger startup: %1\n").arg(
                    QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                );
    lfStream << QString("=======================================\n");
}

/** stream destructor.
 *
 * appends stream with new line,
 * flushes stream,
 * closes log file
 */
FileAppender::~FileAppender()
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    if ( valid )
    {
#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " append log file with new line"
                << std::endl;
#endif
        lfStream << "\n";
        lfStream.flush();

#if LQTL_ENABLE_LOGGER_LOGGING
        std::clog << FUNCTION_NAME
                << " close log file"
                << std::endl;
#endif
        logfile.close();
    }
}

/** log writer implementation
 *
 * appends stream with passed log message and
 * flushes stream
 *
 * @param message log message
 *
 * @return true if FileAppender#valid is set<br>
 *         false otherwise
 */
bool FileAppender::writeLog( QString& message )
{
#if LQTL_ENABLE_LOGGER_LOGGING
    std::clog << FUNCTION_NAME << std::endl;
#endif

    if ( valid )
    {
        lfStream << message << "\n";
        lfStream.flush();

        return true;
    }

    return false;
}

