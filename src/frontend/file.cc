#include "file.hpp"

void File::open(std::string path)
{
        if (this->buffer.is_open())
                this->buffer.close();

        this->buffer.open(path, std::ifstream::in);
        this->is_finished = false;

        this->loc.filename = path;
        this->loc.offset   = 0;
        this->loc.line     = 0;
        this->loc.column   = 0;
}

void File::reset()
{
        this->is_finished = false;
        this->buffer.seekg(0, std::ios::beg);
}

int File::get_char()
{
        if (this->is_finished || !this->buffer.is_open())
                return EOF;

        if (this->buffer.peek() == EOF) {
                this->is_finished = true;
                return EOF;
        }

        int ch = this->buffer.get();
        this->loc.offset++;
        this->loc.column++;
        if (ch == '\n') {
                this->loc.column = 0;
                this->loc.line++;
        }

        return ch;
}

Location File::unknown_location()
{
        Location loc;
        loc.filename = "unknown";
        loc.offset   = -1;
        loc.line     = -1;
        loc.column   = -1;
        return loc;
}
