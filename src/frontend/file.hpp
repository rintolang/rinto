#ifndef RIN_FILE_HPP
#define RIN_FILE_HPP

#include <fstream>
#include <string>
#include "backend.hpp"

#ifndef EOF
#define EOF (-1)
#endif /* EOF */

class File
{
public:
        File(){}
        File(std::string path) : path(path) {open(path);};
        ~File() {if (buffer.is_open()) buffer.close();}

        // set source to path
        void open(std::string path);

        bool is_open()
        { return this->buffer.is_open(); }

        bool has_next()
        { return (!this->is_finished && this->is_open()); }

        Location get_loc() const
        { return this->loc; }

        // restart reading file
        void reset();

        // return next char (-1 if EOF)
        int get_char();

        // return next char without affecting buffer (-1 if EOF)
        int peek()
        { return this->buffer.peek(); }

        static Location unknown_location();

private:
        std::string path = "";
        std::ifstream buffer;
        bool is_finished = false;
        Location loc;
};

#endif /* RIN_FILE_H */
