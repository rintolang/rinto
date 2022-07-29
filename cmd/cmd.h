#ifndef CMD_H
#define CMD_H
#include <map>
#include <string>
#include <iostream>


/*
  rinto [Parameters=[value], flag] file1.rin file2.rin 

    Flags
     -S = assembly
     -h
     --help
     -v = version 

    Parameters
      -o = name output file
    Target
      x86
      x64
      ARM
*/

/*
tokenize argv
generate AST
parse AST
generate C++ code
create executable
*/

struct Flag{
	std::string flag[2];
	std::string help;
	bool takesValue;
};

struct Target{
  std::string type;
  bool intel;
};

void printHelp();
void parseArgv();

#endif