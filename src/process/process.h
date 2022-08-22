#ifndef PROCESS_H
#define PROCESS_H
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
using namespace std;

void process_command(const string &command);
vector<string> delimitSplit(const string& str, const char& ch);

#endif