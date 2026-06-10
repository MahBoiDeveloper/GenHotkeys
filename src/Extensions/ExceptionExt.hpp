#pragma once
#include <exception>
#include <string>
#include <QString>

class ExceptionExt : public std::exception
{
private:
    std::string message;
public:
    ExceptionExt(const std::string& msg) : message{msg}
    {
    }

    ExceptionExt(const char* msg) : message{std::string(msg)}
    {
    }

    ExceptionExt(const QString& msg) : message{msg.toStdString()}
    {
    }

    /// @brief Get normal text context of the error 
    inline const char* what() const noexcept override
    {
        return message.c_str();
    }
};
