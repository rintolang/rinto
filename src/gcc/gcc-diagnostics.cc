/*
 * Implements frontend/diagnostic.hpp.
 * Modified from GCC source: gcc/go/go-gcc-diagnostics.cc.
 *
 * GCC implementation of go diagnostics interface.
 * Copyright (C) 2016-2022 Free Software Foundation, Inc.
 * Contributed by Than McIntosh, Google.
 *
 * This file is part of GCC.
 *
 * GCC is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3, or (at your option) any later
 * version.
 *
 * GCC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GCC; see the file COPYING3.  If not see
 * <http://www.gnu.org/licenses/>.
 */

#include "gcc-backend.hpp"

const char open_quo[]  = "'";
const char close_quo[] = "'";

void rin_be_error_at(const Location location, const std::string& errmsg)
{
        location_t gcc_loc = gcc_location(location);
        error_at(gcc_loc, "%s", errmsg.c_str());
}

void rin_be_warning_at
(const Location location, int opt, const std::string& warningmsg)
{
        location_t gcc_loc = gcc_location(location);
        warning_at(gcc_loc, opt, "%s", warningmsg.c_str());
}

void rin_be_fatal_error
(const Location location, const std::string& fatalmsg)
{
        location_t gcc_loc = gcc_location(location);
        fatal_error(gcc_loc, "%s", fatalmsg.c_str());
}

void rin_be_inform
(const Location location, const std::string& infomsg)
{
        location_t gcc_loc = gcc_location(location);
        inform(gcc_loc, "%s", infomsg.c_str());
}

void rin_be_get_quotechars(const char** open_quote, const char** close_quote)
{
        *open_quote =  &open_quo[0];
        *close_quote = &close_quo[0];
}
