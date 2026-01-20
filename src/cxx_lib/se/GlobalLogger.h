/*
 * filename：GlobalLogger
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Global logger for the programm.
 */
#ifndef IMMEDIATERENDERERVK_GLOBALLOGGER_H
#define IMMEDIATERENDERERVK_GLOBALLOGGER_H
#include <string>
#include "Singleton.h"
#include <fstream>
#include <iostream>

namespace se {
    class GlobalLogger :public Singleton<GlobalLogger> {
        friend class Singleton<GlobalLogger>;
    private:
        std::ofstream log_file;
        std::ostream& clog = std::clog;
        GlobalLogger();
        ~GlobalLogger();
    public:
        void logInfo(const std::string& message) const;
        void logWarning(const std::string& message) const;
        void logError(const std::string& message) const;
    };
} // se

#endif //IMMEDIATERENDERERVK_GLOBALLOGGER_H