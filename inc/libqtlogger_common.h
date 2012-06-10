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

#include <QtCore/qglobal.h>

namespace ilardm {
namespace lib {
namespace qtlogger {

#if defined(LIBQTLOGGER_LIBRARY)
#  define LIBQTLOGGER_EXPORT Q_DECL_EXPORT
#else
/** Qt define for exporting symbols for shared libabry
 */
#  define LIBQTLOGGER_EXPORT Q_DECL_IMPORT
#endif

/** enables/disables console logging of logger itself.
 *
 * disabled in Release builds
 */
#define LQTL_ENABLE_LOGGER_LOGGING   (1)

#if     defined( _RELEASE )
#undef  LQTL_ENABLE_LOGGER_LOGGING
#define LQTL_ENABLE_LOGGER_LOGGING   (0)     // forced no debug in release
#endif

#ifndef __GNUC__
#ifdef  _MSC_VER
#define FUNCTION_NAME           __FUNCSIG__
#else
/** compiler-specific function signature define.
 *
 * wrapper for __func__ as fallback,
 * __PRETTY_FUNCTION__ if GCC,
 * __FUNCSIG__ if MSVC
 */
#define FUNCTION_NAME           __func__
#endif
#else
#define FUNCTION_NAME           __PRETTY_FUNCTION__
#endif

}   // qtlogger
}   // lib
}   // ilardm
