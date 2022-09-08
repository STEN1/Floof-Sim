#pragma once

namespace FLOOF {
    namespace Utils {
        class Logger {
        public:
            inline static Logger* s_Logger = nullptr;
            Logger(const char* logfile);
           ~Logger();
           enum LogType{
               INFO = 0,
               WARNING,
               ERROR,
               CRITICAL,
               };
           void log(LogType logtype, const char* message);
        private:
           const char* m_LogPath;
        };
    }
}
