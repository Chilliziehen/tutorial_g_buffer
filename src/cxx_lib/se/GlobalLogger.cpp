/*
 * filename：GlobalLogger
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Singleton global logger implementation.
 */

#include "GlobalLogger.h"
#include "config.h"
#include "CLI_Color.h"

namespace se {
    GlobalLogger::GlobalLogger() {
        // Initialize logger, e.g., open log file in LOGS_DIR
        const auto logs_dir = std::string(LOGS_DIR);
        const auto log_filename = std::string(LOG_FILENAME);
        // Ensure the directory exists or create it if necessary
        log_file.open(logs_dir + "/" + log_filename, std::ios::out | std::ios::app);
        if (!log_file.is_open()) {
            throw std::runtime_error("[Warning]Failed to open log file: " + logs_dir + "/" + log_filename);
        }
        this->clog.rdbuf(log_file.rdbuf());
        if constexpr (!BUFFERED_LOGS)
            this->clog.setf(std::ios::unitbuf);// unbuffered
    }
    GlobalLogger::~GlobalLogger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }
    void GlobalLogger::logInfo(const std::string& message) const {
        this->clog << "[INFO] " << message << std::endl;
        std::cout<< green << "[INFO] " << white << message <<reset<< std::endl;
    }
    void GlobalLogger::logWarning(const std::string& message) const {
        this->clog << "[WARNING] " << message << std::endl;
        std::cout<< yellow << "[WARNING] " << white << message <<reset<< std::endl;
    }
    void GlobalLogger::logError(const std::string& message) const {
        this->clog << "[ERROR] " << message << std::endl;
        std::cout<< red << "[ERROR] " << white << message <<reset<< std::endl;
    }
} // se