//
// Created by dPons on 3/20/19.
//

#pragma once

#ifndef OCTOSPORK_LOGGER_H
#define OCTOSPORK_LOGGER_H

#include <fstream>

using namespace std;

class Logger {

private:
    string LastMessage;

    string FileName;

    FILE * LogFile;

    bool print;
public:

    Logger(bool print = false);

    Logger(const string &FileName, bool print=false);

    string getLastMessage() const;

    void WriteLog(string ToWrite);

    ~Logger();
};


#endif //OCTOSPORK_LOGGER_H
