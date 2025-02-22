#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include <string>

/*Macro to print a message with the LOG_INFO serverity. This is a simple convenience function*/
#define MSG_INFO(msg)    printLogMessage(LOG_INFO,    "[INFO] "       + std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + msg)
/*Macro to print a message with the LOG_WARNING serverity. This is a simple convenience function*/
#define MSG_WARNING(msg) printLogMessage(LOG_WARNING, "[WARNING] "    + std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + msg)
/*Macro to print a message with the LOG_INFO serverity. This is a simple convenience function*/
#define MSG_ERROR(msg)   printLogMessage(LOG_ERROR,   "[ERROR] "      + std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + msg)
/*Macro to print a message with the LOG_FATAL serverity. This is a simple convenience function*/
#define MSG_FATAL(msg)   printLogMessage(LOG_FATAL,   "[FATAL ERROR] " + std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + msg)

/**
 * @brief Represents a serverity of a message
 */
enum LogMessageServerity
{
    /*Simple information; E.g. new states, positions, ...*/
    LOG_INFO,
    /*A warning; Should not be used for errors but when things are not quite working as expected*/
    LOG_WARNING,
    /*Error: Something went wrong, but its not critical*/
    LOG_ERROR,
    /*Fatal Error: Something went extremely wrong*/
    LOG_FATAL
};

/**
 * @brief Function which prints a log message (string) with a specific serverity
 * @param pServerity The serverity of the message
 * @param pMessage The message string to display
 */
void printLogMessage(LogMessageServerity pServerity, std::string pMessage);

#endif /*LOGGER_HPP_INCLUDED*/