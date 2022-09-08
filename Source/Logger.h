//
// Created by Adrian Drevland on 08/09/2022.
//

#ifndef FLOOF_LOGGER_H
#define FLOOF_LOGGER_H

namespace Utils{
class Logger {
public:
    Logger(const char* logfile);
   ~Logger();
   enum LogType{
       WARNING = 0,
       ERROR,
       CRITICAL,
       };
   void  log(const char* message);
   void log(LogType logtype, const char* message);
private:
   const char* m_LogPath;

};
}

#endif //FLOOF_LOGGER_H
