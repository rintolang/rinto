/*
 * Modified from the Go programming language frontend.
 * https://github.com/golang/gofrontend
 *
 * Copyright (c) 2009 The Go Authors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RIN_DIAGNOSTICS_HPP
#define RIN_DIAGNOSTICS_HPP

#include <sstream>
#include <cstdarg>
#include <string>
#include <cstdio>

#include "backend.hpp"
#include "file.hpp"

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#define RIN_ATTRIBUTE_GCC_DIAG(m, n) __attribute__ ((__format__ (__gcc_tdiag__, m, n))) __attribute__ ((__nonnull__ (m)))
#else
#define RIN_ATTRIBUTE_GCC_DIAG(m,  n)
#endif

#if __GNUC__ >= 3
#define RIN_ATTRIBUTE_PRINTF(m, n) __attribute__ ((__format__ (__printf__, m, n))) __attribute__ ((__nonnull__ (m)))
#else
#define RIN_ATTRIBUTE_PRINTF(m, n)
#endif

/*
 * These declarations define the interface through which the frontend
 * reports errors and warnings. These functions accept printf-like
 * format specifiers (e.g. %d, %f, %s, etc), with the following additional
 * extensions:
 *
 *  1.  'q' qualifier may be applied to a specifier to add quoting, e.g.
 *      %qd produces a quoted decimal output, %qs a quoted string output.
 *      [This extension is supported only with single-character format
 *      specifiers].
 *  2.  %m specifier outputs value of "strerror(errno)" at time of call.
 *
 *  3.  %< outputs an opening quote, %> a closing quote.
 *
 * All other format specifiers are as defined by 'sprintf'. The final resulting
 * message is then sent to the back end via rin_be_error_at/rin_be_warning_at.
 */
extern void rin_error_at(const Location, const char* fmt, ...)
        RIN_ATTRIBUTE_GCC_DIAG(2,3);
extern void rin_warning_at(const Location, int opt, const char* fmt, ...)
        RIN_ATTRIBUTE_GCC_DIAG(3,4);
extern void rin_fatal_error(const Location, const char* fmt, ...)
        RIN_ATTRIBUTE_GCC_DIAG(2,3);
extern void rin_inform(const Location, const char* fmt, ...)
        RIN_ATTRIBUTE_GCC_DIAG(2,3);

/*
 * rin_debug is used to report a debugging message at a location.  This
 * uses standard printf formatting.
 */
extern void rin_debug(const Location, const char* fmt, ...)
        RIN_ATTRIBUTE_PRINTF(2, 3);

/*
 * These interfaces provide a way for the front end to ask for
 * the open/close quote characters it should use when formatting
 * diagnostics (warnings, errors).
 */
extern const char* rin_open_quote();
extern const char* rin_close_quote();

/*
 * These interfaces are used by utilities above to pass warnings and
 * errors (once format specifiers have been expanded) to the back end,
 * and to determine quoting style. Avoid calling these routines directly;
 * instead use the equivalent routines above. The back end is required to
 * implement these routines.
 */
extern void rin_be_error_at(const Location, const std::string& errmsg);
extern void rin_be_warning_at(const Location, int opt, const std::string& warningmsg);
extern void rin_be_fatal_error(const Location, const std::string& errmsg);
extern void rin_be_inform(const Location, const std::string& infomsg);
extern void rin_be_get_quotechars(const char** open_quote, const char** close_quote);

#endif // RIN_DIAGNOSTICS_HPP
