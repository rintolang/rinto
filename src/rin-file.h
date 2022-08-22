#ifndef RIN_FILE_H
#define RIN_FILE_H

#include <fstream>
#include <string>
#include <queue>
#include <exception>

#ifndef EOF
#define EOF (-1)
#endif /* EOF */

/* Position keeps track of a position in a file. */
typedef struct {
        std::string filename;
        int         offset;
        int         line;
        int         column;

} position;

class RinFile
{
private:
        std::ifstream           Buffer;
        std::queue<std::string> Files;
        bool                    Finished;

        std::string Filename = "";
        int         Offset   = 0;
        int         Line     = 0;
        int         Column   = 0;
public:
        RinFile(){};

        /* Construct and immediately open a file  */
        RinFile(std::string path);

        /* Closes Buffer if open. */
        ~RinFile();

        /* Get position */
        position pos();

        /* Immediately reads a file to buffer */
        std::string open(std::string path);

        /* Opens the next file in queue and returns the file's path */
        std::string next();

        /* Returns true if buffer returned EOF. */
        bool eof();

        /* Returns true if there are more files in queue. */
        bool hasNext();

        /* Queue a file path. */
        void addFile(std::string path);

        /* Get character from current buffer.*/
        int getChar();

        /* Peek the next character without affecting the buffer */
        int peek();
};

#endif /* RIN_FILE_H */
