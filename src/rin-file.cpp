#include "rin-file.h"

RinFile::~RinFile()
{
        if (Buffer.is_open())
                Buffer.close();
}

RinFile::RinFile(std::string path) {
        open(path);
}

std::string RinFile::open(std::string path)
{
        if (Buffer.is_open())
                Buffer.close();

        Buffer.open(path, std::ifstream::in);
        Finished = false;

        Filename = path;
        Line = Offset = Column = 0;

        return path;
}

std::string RinFile::next()
{
        if (Files.empty())
                return "";

        std::string f = Files.front();
        Files.pop();

        return open(f);
}

bool RinFile::eof() {
        return Finished;
}

bool RinFile::hasNext() {
        return (!Files.empty() || !Finished);
}

void RinFile::addFile(std::string path) {
        Files.push(path);
}

int RinFile::getChar()
{
        if (Finished) return -1;
        int ch = Buffer.get();

        Offset++;
        Column++;

        if (ch == '\n') {
                Column = 0;
                Line++;
        }

        if (Buffer.peek() == EOF || ch == EOF)
                Finished = true;

        return ch;
}

int RinFile::peek() {
        return Buffer.peek();
}

position RinFile::pos()
{
        return (position) {
                Filename,
                Offset,
                Line,
                Column
        };
}
