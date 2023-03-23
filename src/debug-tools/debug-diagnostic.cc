#include <diagnostic.hpp>

const char open_quote[]  = "'";
const char close_quote[] = "'";

void rin_be_error_at(const Location loc, const std::string& errmsg)
{
        printf("[DEBUG ERROR] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &errmsg[0]);
}

void rin_be_warning_at(const Location loc, int opt, const std::string& warningmsg)
{
        printf("[DEBUG WARNING] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &warningmsg[0]);
}

void rin_be_fatal_error(const Location loc, const std::string& errmsg)
{
        printf("[DEBUG FATAL] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &errmsg[0]);
}

void rin_be_inform(const Location loc, const std::string& infomsg)
{
        printf("[DEBUG INFORM] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &infomsg[0]);
}

void rin_be_get_quotechars(const char** open_quo, const char** close_quo)
{
        *open_quo =  &open_quote[0];
        *close_quo = &close_quote[0];
}
