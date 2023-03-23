/*
 * Modified from the Go programming language frontend.
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
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESSf FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "diagnostic.hpp"

static std::string mformat_value() 
{ return std::string(strerror(errno)); }

/*
 * Rewrite a format string to expand any extensions not
 * supported by sprintf(). See comments in diagnostics.hpp
 * for list of supported format specifiers.
 */
static std::string expand_format(const char* fmt)
{
        std::stringstream ss;
        for (const char* c = fmt; *c; ++c) {
                if (*c != '%') {
                        ss << *c;
                        continue;
                }
                c++;
                switch (*c) {
                case '\0': {
                        // malformed format string
                        break;
                }
                case '%': {
                        ss << "%";
                        break;
                }
                case 'm': {
                        ss << mformat_value();
                        break;
                }
                case '<': {
                        ss << rin_open_quote();
                        break;
                }
                case '>': {
                        ss << rin_close_quote();
                        break;
                }
                case 'q': {
                        ss << rin_open_quote();
                        c++;
                        if (*c == 'm') {
                                ss << mformat_value();
                        } else {
                                ss << "%" << *c;
                        }
                        ss << rin_close_quote();
                        break;
                }
                default: {
                        ss << "%" << *c;
                }
                }
        }

        return ss.str();
}

/*
 * Expand message format specifiers, using a combination of
 * expand_format above to handle extensions (ex: %m, %q) and vasprintf()
 * to handle regular printf-style formatting. A pragma is being used here to
 * suppress this warning:
 *
 *   warning: function ‘std::__cxx11::string expand_message(const char*, __va_list_tag*)’ might be a candidate for ‘gnu_printf’ format attribute [-Wsuggest-attribute=format]
 *
 * What appears to be happening here is that the checker is deciding that
 * because of the call to vasprintf() (which has attribute gnu_printf), the
 * calling function must need to have attribute gnu_printf as well, even
 * though there is already an attribute declaration for it.
 */
static std::string expand_message(const char* fmt, va_list ap)
        RIN_ATTRIBUTE_GCC_DIAG(1,0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"

static std::string expand_message(const char* fmt, va_list ap)
{
        char* mbuf = 0;
        std::string expanded_fmt = expand_format(fmt);
        int nwr = vasprintf(&mbuf, expanded_fmt.c_str(), ap);
        if (nwr == -1)
        {
                // memory allocation failed
                rin_be_error_at(File::unknown_location(),
                               "memory allocation failed in vasprintf");
        }
        std::string rval = std::string(mbuf);
        free(mbuf);
        return rval;
}

#pragma GCC diagnostic pop

static const char* cached_open_quote  = NULL;
static const char* cached_close_quote = NULL;

const char* rin_open_quote()
{
        if (cached_open_quote == NULL)
                rin_be_get_quotechars(&cached_open_quote, &cached_close_quote);
        return cached_open_quote;
}

const char* rin_close_quote()
{
        if (cached_close_quote == NULL)
                rin_be_get_quotechars(&cached_open_quote, &cached_close_quote);
        return cached_close_quote;
}

void rin_error_at(const Location loc, const char* fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        rin_be_error_at(loc, expand_message(fmt, ap));
        va_end(ap);
}

void rin_warning_at(const Location loc, int opt, const char* fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        rin_be_warning_at(loc, opt, expand_message(fmt, ap));
        va_end(ap);
}

void rin_fatal_error(const Location loc, const char* fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        rin_be_fatal_error(loc, expand_message(fmt, ap));
        va_end(ap);
}

void rin_inform(const Location loc, const char* fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        rin_be_inform(loc, expand_message(fmt, ap));
        va_end(ap);
}

// rin_debug uses normal printf formatting, not GCC diagnostic formatting.
void rin_debug(const Location loc, const char* fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        char* mbuf = NULL;
        int nwr = vasprintf(&mbuf, fmt, ap);
        va_end(ap);
        if (nwr == -1) {
                rin_be_error_at(File::unknown_location(),
                        "memory allocation failed in vasprintf");
        }
        std::string rval = std::string(mbuf);
        free(mbuf);
        rin_be_inform(loc, rval);
}

