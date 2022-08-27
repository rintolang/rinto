#ifndef CMD_H
#define CMD_H

#include "scanner.h"

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

struct Flag{
        std::string              flag[2];
        std::string              help;
        bool                     takesValue;
};

struct Target{
        std::string              type;
        bool                     intel;
};

typedef struct {
        std::string              Cmd;
        std::vector<std::string> Flags;
} ArgCmd;

typedef struct {
        std::string              Name;
        std::string              ShortHelp;
        std::string              LongHelp;
} CmdHelp;

private:
        std::string VersionStr = "0.0.1";
        std::string HelpStr;

        ArgCmd Arg;
        std::vector<CmdHelp> Commands;

        /*
         * Parses argv.
         * TODO:
         * 1. Arguments should be whitespace seperated, meaning you should
         *    probably write a separate function to skip \n, space and \t
         *    characters until it finds an alphanumeric char.
         * 2. Once the first alphanum char is detected, read it unti you
         *    encounter whitespace again (ensure that the whitespace char
         *    is not included) and isolate the word into its own string
         *    var.
         * 3. If the word doesnt start with a "--", then it is a command.
         *    If you detect a command, create an ArgCmd and set Cmd to
         *    the string of the word you just encountered.
         * 4. If the word starts with a '--', then it is a flag. Gather
         *    each flag and insert it into your ArgCmd's Flags vector.
         *    Note that flags are options for commands, so any flag following
         *    a command is added to that command's ArgCmd structure. If a flag
         *    is detected before the first command, then you print help as
         *    this is an error.
         * 5. Set your detected command to the CMD attriute 'Arg'. Note that
         *    each call to the cmd program should only be 1 command. So if
         *    multiple commands are detected then you have an error.
         */
        void parseArgs();

        /*
         * TODO:
         * Once args are parsed, execArgs should handle execution of the user's
         * command and flags. This should be a switch statement on Arg.Cmd. If
         * the command is unknown, printHelp().
         */
        void execArgs();

        /*
         * TODO
         * 1. Print the generic help text (HelpStr).
         * 2. Iterate through Commands and print their name
         *     and ShortHelp texts.
         */
        void printHelp();

        // Processes commands related to the scanner package. You may ignore this for now.
        void pScanner();
public:
        /*
         * TODO
         * 1. Initialize HelpStr
         * 2. Initialize Commands vector.
         */
        CMD(int argc, char* argv[]);
};


void process_command(const string &command);
vector<string> delimitSplit(const string& str, const char& ch);
void initTarget();
void initFlags();
void printHelp();
// TODO: void parseArgv();

#endif