//
// Created by dPons on 3/20/19.
//

#include "Logger.h"
#include <chrono>
#include <stdio.h>

using namespace std;
using namespace chrono;

Logger::Logger() {
    // Get current time
    auto CurrentTime = system_clock::to_time_t(system_clock::now());

    // Format current time as Day/Month/DayNr_hh:dd:ss:Yr.Log
    auto FormattedTime = std::ctime(&CurrentTime);
    FileName.assign("../Logs/").append(FormattedTime).erase(FileName.end()-1, FileName.end());
    FileName += ".Log";

    // Replace spaces with underscores
    for (char &c : FileName) {
        c = c == ' ' ? '_' : c;
    }

    // Open log file
    LogFile = fopen(FileName.data(), "w+");
}

Logger::Logger(const string &FileName){
    this->FileName.assign("../Logs/");
    this->FileName.append(FileName).append(".Log");

    // Replace spaces with underscores
    for (char &c : this->FileName) {
        c = c == ' ' ? '_' : c;
    }

    // Open log file
    LogFile = fopen(this->FileName.data(), "w+");
}

Logger::~Logger() {
    fclose(LogFile);
}

string Logger::getLastMessage() const {
    return string(LastMessage);
}

void Logger::WriteLog(string ToWrite) {
    LastMessage.assign(ToWrite);

    // Get current time
    auto CurrentTime = system_clock::to_time_t(system_clock::now());
    // Format current time as Day/Month/DayNr_hh:dd:ss:Yr.Log
    auto FormattedTime = string(std::ctime(&CurrentTime));
    FormattedTime.erase(FormattedTime.end()-1, FormattedTime.end());

    fprintf(LogFile, "[%s]: %s\n", FormattedTime.data(), ToWrite.data());
}

