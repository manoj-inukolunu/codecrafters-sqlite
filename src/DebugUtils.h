#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <iostream>
#include <sstream>

// Uncomment the following line to enable debug output
#define ENABLE_DEBUG_LOGGING

// Log levels
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// Base logging function (internal use)
void logMessage(LogLevel level, const std::string& message);

// Debug logging macros
#ifdef ENABLE_DEBUG_LOGGING
    #define LOG_DEBUG(msg) do { \
        std::ostringstream oss; \
        oss << msg; \
        logMessage(LogLevel::DEBUG, oss.str()); \
    } while(0)
    
    #define LOG_INFO(msg) do { \
        std::ostringstream oss; \
        oss << msg; \
        logMessage(LogLevel::INFO, oss.str()); \
    } while(0)
    
    #define LOG_WARNING(msg) do { \
        std::ostringstream oss; \
        oss << msg; \
        logMessage(LogLevel::WARNING, oss.str()); \
    } while(0)
    
    #define LOG_ERROR(msg) do { \
        std::ostringstream oss; \
        oss << msg; \
        logMessage(LogLevel::ERROR, oss.str()); \
    } while(0)
#else
    #define LOG_DEBUG(msg) ((void)0)
    #define LOG_INFO(msg) ((void)0)
    #define LOG_WARNING(msg) ((void)0)
    #define LOG_ERROR(msg) ((void)0)
#endif

// For backward compatibility with existing code
#ifdef ENABLE_DEBUG_LOGGING
    #define DBG_OUT std::cerr
#else
    #define DBG_OUT if (false) std::cerr
#endif

#endif // DEBUG_UTILS_H
