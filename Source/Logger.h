#pragma once

namespace FLOOF {
    namespace Utils {
        class Logger {
        public:
            inline static Logger* s_Logger = nullptr;
            Logger(const char* logfile);
           ~Logger();
           enum LogType {
               WARNING = 0,
               ERROR,
               CRITICAL,
           };
           void log(const char* message);
           void log(LogType logtype, const char* message);
        private:
           const char* m_LogPath;
        };
    }
}
