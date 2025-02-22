#include "logger.hpp"
#include <iostream>
#include <mutex>

std::mutex terminalMutex;

enum Color
{


    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};
enum Effect
{
    EFFECT_BOLD,
    EFFECT_UNDERLINE,
    EFFECT_INVERSE
};

void setForegroundColor(std::ostream& pStream, Color pColor)
{
    switch(pColor)
    {
        case COLOR_BLACK:
        {
            pStream << "\033[0;30m";
        }
        break;
        case COLOR_RED:
        {
            pStream << "\033[0;31m";
        }
        break;
        case COLOR_GREEN:
        {
            pStream << "\033[0;32m";
        }
        break;
        case COLOR_YELLOW:
        {
            pStream << "\033[0;33m";
        }
        break;
        case COLOR_BLUE:
        {
            pStream << "\033[0;34m";
        }
        break;
        case COLOR_MAGENTA:
        {
            pStream << "\033[0;35m";
        }
        break;
        case COLOR_CYAN:
        {
            pStream << "\033[0;36m";
        }
        break;
        case COLOR_WHITE:
        {
            pStream << "\033[0;37m";
        }
        break;
        default:
        {
            throw(std::invalid_argument("Invalid color passed to setForegroundColor()!"));
        }
    }
}
void setBackgroundColor(std::ostream& pStream, Color pColor)
{
    switch(pColor)
    {
        case COLOR_BLACK:
        {
            pStream << "\033[0;40";
        }
        break;
        case COLOR_RED:
        {
            pStream << "\033[0;41m";
        }
        break;
        case COLOR_GREEN:
        {
            pStream << "\033[0;42m";
        }
        break;
        case COLOR_YELLOW:
        {
            pStream << "\033[0;43m";
        }
        break;
        case COLOR_BLUE:
        {
            pStream << "\033[0;44m";
        }
        break;
        case COLOR_MAGENTA:
        {
            pStream << "\033[0;45m";
        }
        break;
        case COLOR_CYAN:
        {
            pStream << "\033[0;46m";
        }
        break;
        case COLOR_WHITE:
        {
            pStream << "\033[0;47m";
        }
        break;
        default:
        {
            throw(std::invalid_argument("Invalid color passed to setBackgroundColor()!"));
        }
    }
}
void resetTerminalStyle(std::ostream& pStream)
{
    pStream << "\033[0m";
}
void setTerminalEffect(std::ostream& pStream, Effect pEffect)
{
    switch(pEffect)
    {
        case EFFECT_BOLD:
        {
            pStream << "\033[1m";
        }
        break;
        case EFFECT_UNDERLINE:
        {
            pStream << "\033[4m";
        }
        break;
        case EFFECT_INVERSE:
        {
            pStream << "\033[7m";
        }
        break;
        default:
        {
            throw(std::invalid_argument("Invalid effect passed to setTerminalEffect()!"));
        }
    }
}
void removeTerminalEffect(std::ostream& pStream, Effect pEffect)
{
    switch(pEffect)
    {
        case EFFECT_BOLD:
        {
            pStream << "\033[21m";
        }
        break;
        case EFFECT_UNDERLINE:
        {
            pStream << "\033[24m";
        }
        break;
        case EFFECT_INVERSE:
        {
            pStream << "\033[27m";
        }
        break;
        default:
        {
            throw(std::invalid_argument("Invalid effect passed to setTerminalEffect()!"));
        }
    }
}

const std::string redForeground("\033[0;31m");
const std::string reset("\033[0m");

void printLogMessage(LogMessageServerity pServerity, std::string pMessage)
{
    //terminalMutex.lock();
    switch(pServerity)
    {
        case LOG_INFO:
        {
            setForegroundColor(std::cout, COLOR_BLUE);
        }
        break;
        case LOG_WARNING:
        {
            setForegroundColor(std::cout, COLOR_YELLOW);
        }
        break;
        case LOG_ERROR:
        {
            setTerminalEffect(std::cout, EFFECT_BOLD);
            setForegroundColor(std::cout, COLOR_RED);
        }
        break;
        case LOG_FATAL:
        {
            setTerminalEffect(std::cout, EFFECT_BOLD);
            setTerminalEffect(std::cout, EFFECT_UNDERLINE);
            setForegroundColor(std::cout, COLOR_RED);
        }
        break;
        default:
        {
            terminalMutex.unlock();
            throw(std::invalid_argument("Invalid log level passed to printLogMessage()!"));
        }
    }
    std::cout << pMessage << std::endl;
    resetTerminalStyle(std::cout);
    //terminalMutex.unlock();
}