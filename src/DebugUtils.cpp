#include "DebugUtils.h"
#include <iomanip>
#include <ctime>

void logMessage(LogLevel level, const std::string& message) {
    // Get current time
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    // Output timestamp
    std::cout << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
    
    // Output log level
    switch (level) {
        case LogLevel::DEBUG:   std::cout << "[DEBUG]   "; break;
        case LogLevel::INFO:    std::cout << "[INFO]    "; break;
        case LogLevel::WARNING: std::cout << "[WARNING] "; break;
        case LogLevel::ERROR:   std::cout << "[ERROR]   "; break;
    }
    
    // Output the message
    std::cout << message << std::endl;
}
