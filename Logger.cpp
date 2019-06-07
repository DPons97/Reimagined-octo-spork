//
// Created by dPons on 3/20/19.
//

#include "Logger.h"
#include <chrono>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace chrono;

/**
 * Logger class that stores in file all prints
 * REMEMBER: use this class as a pointer and delete manually when no more used. Otherwise log file is not saved on disk.
 * @param print True if text has to be printed in console
 */
Logger::Logger(bool print) {
    LogFile = new fstream();
    // Get current time
    auto CurrentTime = system_clock::to_time_t(system_clock::now());

    // Format current time as Day/Month/DayNr_hh:dd:ss:Yr.Log
    auto FormattedTime = std::ctime(&CurrentTime);
    FileName.assign("Logs/").append(FormattedTime).erase(FileName.end()-1, FileName.end());
    FileName += ".log";

    // Replace spaces with underscores
    for (char &c : FileName) {
        c = c == ' ' ? '_' : c;
    }

    // Try to open Logs directory
    DIR * LogDir = opendir("Logs/");
    if (LogDir == nullptr) {
        // Create new dir
        mkdir("Logs/", 0777);
    } else closedir(LogDir);

    // Open log file
    LogFile->open(FileName.data(), ios::out);

    this->print = print;
}

/**
 * Logger class that stores in file all prints
 * REMEMBER: use this class as a pointer and delete manually when no more used. Otherwise log file is not saved on disk.
 * @param fileName custom name of log file
 * @param print True if text has to be printed in console
 */
Logger::Logger(const string &fileName, bool print){
    LogFile = new fstream();

    this->FileName.assign("Logs/");
    this->FileName.append(fileName).append(".log");

    // Replace spaces with underscores
    for (char &c : this->FileName) {
        c = c == ' ' ? '_' : c;
    }

    // Try to open Logs directory
    DIR * LogDir = opendir("Logs/");
    if (LogDir == nullptr) {
        // Create new dir
        mkdir("Logs/", 0777);
    } else closedir(LogDir);


    // Open log file
    LogFile->open(this->FileName.data(), ios::out);
    this->print = print;
}

/**
 * Retreive this logger last message
 * @return last message that was written by this logger
 */
string Logger::getLastMessage() const {
    return string(LastMessage);
}

/**
 * Write stuff to log/console
 * @param ToWrite string to write
 */
void Logger::writeLog(const string &ToWrite) {
    LastMessage.assign(ToWrite);

    // Get current time
    auto CurrentTime = system_clock::to_time_t(system_clock::now());
    // Format current time as Day/Month/DayNr_hh:dd:ss:Yr.Log
    auto FormattedTime = string(std::ctime(&CurrentTime));
    FormattedTime.erase(FormattedTime.end()-1, FormattedTime.end());

    string toWrite;
    toWrite.assign("[").append(FormattedTime).append("]: ").append(ToWrite).append("\n");
    LogFile->write(toWrite.data(), toWrite.length());

    if (print) printf("[%s]: %s\n", FormattedTime.data(), ToWrite.data());
}

Logger::~Logger() {
    if (LogFile->is_open())
        LogFile->close();
}
