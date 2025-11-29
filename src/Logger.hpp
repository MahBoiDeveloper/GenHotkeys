#pragma once
#include <sstream>
#include <fstream>
#include <memory>

#include "StringExt.hpp"

#define LOGGER      Logger::Instance
#define LOGSTM      Logger::Instance->Log()
#define LOGMSG(msg) Logger::Instance->Log(StringExt::EmptyString+msg);Logger::Instance->LogToConsole(StringExt::EmptyString+msg)

#define MAX_LOGS_COUNT 10

class Logger final
{
public: // Data
    inline const static std::unique_ptr<Logger> Instance = std::make_unique<Logger>();
    inline const static QString EXCEPTION_HEADER = "I'VE GOT A PRESENT FOR YA";
private:
    std::ofstream LogFile;

public: // Methods
    Logger();
    ~Logger();

    /// @brief Write [DATE-TIME] and return stream to write other data. Needs to be ended with `endl`
    std::ofstream& Log();
    
    /// @brief Write message to the console.
    void LogToConsole(const char* msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const std::string& msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const QString& msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const std::stringstream& msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const std::wstringstream& msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const std::wstring& msg) const;
    /// @brief Write message to the console.
    void LogToConsole(const wchar_t* msg) const;

    /// @brief Write message to the log file.
    void Log(const char* msg);
    /// @brief Write message to the log file.
    void Log(const std::string& msg);
    /// @brief Write message to the log file.
    void Log(const QString& msg);
    /// @brief Write message to the log file.
    void Log(const std::stringstream& msg);
    /// @brief Write message to the log file.
    void Log(const std::wstringstream& msg);
    /// @brief Write message to the log file.
    void Log(const std::wstring& msg);
    /// @brief Write message to the log file.
    void Log(const wchar_t* msg);

    /// @brief Write text "I'VE GOT A MESSAGE FOR YA" to the log file with text of the exception.
    void LogException(const char* msg);
    const char* GetExceptionHeader();
private:
    /// @brief Write system information from Windows registry to .log file
    void LogSystemInformation();
    /// @brief Get file name like "Logs\\Log YYYY-mm-dd hh-MM-ss.log"
    const std::string GetLogFileName() const;
    /// @brief Get current time in yyyy-MM-dd hh:mm:ss format
    const std::string GetCurrentTime() const;
    /// @brief Returns Windows bit as a string.
    const std::string GetWindowsBit()  const;
};
