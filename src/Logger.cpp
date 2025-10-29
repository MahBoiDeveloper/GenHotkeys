#include <windows.h>
#include <ctime>
#include <filesystem>
#include <vector>
#include <QMessageBox>
#include <QDebug>

#include "Logger.hpp"
#include "Registry.hpp"
#include "Info.hpp"

using namespace std;

#pragma region ctor and dtor
    Logger::Logger()
    {
        // Disable console if it is really opened.
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        
        vector<filesystem::directory_entry> files;
        for (const auto& entry : filesystem::directory_iterator("Logs"))
            if (entry.is_regular_file())
                files.push_back(entry);

        if (files.size() > MAX_LOGS_COUNT)
        {
            sort(files.begin(), files.end(), [](const filesystem::directory_entry& a, const filesystem::directory_entry& b)
            {
                return filesystem::last_write_time(a) > filesystem::last_write_time(b);
            });

            for (auto it = files.begin() + MAX_LOGS_COUNT; it != files.end(); ++it)
            {
                try
                {
                    filesystem::remove(it->path());
                }
                catch (...)
                {
                }
            }
        }

        LogFile.open(GetLogFileName());
        
        // Due to Logger is a singleton, we must create check if folder Logs exists.
        if (!LogFile.is_open())
        {
            filesystem::create_directory("Logs");
            LogFile.open(string("Logs") + GetLogFileName());
        }

        if (!LogFile.is_open()) 
            QMessageBox::critical(nullptr, EXCEPTION_HEADER, "Unable to  create log file; Make sure \"Logs\" folder are exists.");
        
        string title   = "C&C Generals and Generals Zero Hour hotkey editor";
        string version = string("Version: ") + VERSION;
        string authors = string("Authors: ") + AUTHORS;
        Log(title);      LogToConsole(title);
        Log(version);    LogToConsole(version);
        Log(authors);    LogToConsole(authors);
        LogFile << endl; qDebug() << "";

        LogSystemInformation();

        Log(filesystem::current_path().c_str());
    }

    Logger::~Logger()
    {
        if (LogFile.is_open()) LogFile.close();
    }
#pragma endregion

#pragma region Log methods
    void Logger::LogSystemInformation()
    {
        // Write to log all necessary information about MS Windows
        Log() << "Operation System Information"               << endl;
        Log() << "Version   : "
                                << Registry::GetWindowsVersion()              << ' '
                                << GetWindowsBit()                            << endl;
        Log() << "Language  : " << Registry::GetCurrentUserLanguage() << endl << endl;

        // Write to log all information about processor type and memory size
        Log() << "Hardware Information"                         << endl;
        Log() << "Processor : " << Registry::GetProcessorInfo() << endl;
        Log() << "Memory    : " << Registry::GetMemoryInfo()      << endl << endl;

        // Write to log all games paths
        Log() << "Software Information" << endl;

        for (const auto& game : {Registry::Games::Generals, Registry::Games::GeneralsZeroHour})
        {
            if (Registry::GetPathToGame(game).empty())
                Log() << "C&C: " << Registry::ToString(game)  << " not installed" << endl;
            else
                Log() << "C&C: " << Registry::ToString(game)  << " installed at ["
                                 << ToQString(Registry::GetPathToGame(game)).toStdString() << ']' << endl;
        }

        LogFile << endl;
    }

    const string Logger::GetCurrentTime() const
    {
        time_t timeStomp = time(nullptr);
        tm timeNow = {};
        localtime_s(&timeNow, &timeStomp);

        char currentTime[128];
        strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %X", &timeNow);

        stringstream ss;
        ss << currentTime;

        return ss.str();
    }

    const string Logger::GetLogFileName() const
    {
        time_t timeStomp = time(nullptr);
        tm timeNow = {};
        localtime_s(&timeNow, &timeStomp);

        char currentTime[128];
        strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H-%M-%S", &timeNow);

        stringstream ss;
        ss << "Logs\\Log " << currentTime << ".log";

        return ss.str();
    }

    void Logger::LogToConsole(const char* msg)               const { qDebug() << "[" << GetCurrentTime().c_str() << "]\t" << msg; }
    void Logger::LogToConsole(const wchar_t* msg)            const { qDebug() << "[" << GetCurrentTime().c_str() << "]\t" << msg; }
    void Logger::LogToConsole(const std::string& msg)        const { LogToConsole(msg.c_str()); }
    void Logger::LogToConsole(const QString& msg)            const { LogToConsole(msg.toStdString().c_str()); }
    void Logger::LogToConsole(const std::stringstream& msg)  const { LogToConsole(msg.str().c_str()); }
    void Logger::LogToConsole(const std::wstringstream& msg) const { LogToConsole(msg.str().c_str()); }
    void Logger::LogToConsole(const std::wstring& msg)       const { LogToConsole(msg.c_str()); }

    ofstream& Logger::Log()
    {
        LogFile << "[" << GetCurrentTime() << "]\t";
        return LogFile;
    }

    void Logger::Log(const char* msg)          { Log() << msg << endl; }
    void Logger::Log(const string& msg)        { Log(msg.c_str()); }
    void Logger::Log(const QString& msg)       { Log(msg.toStdString()); }
    void Logger::Log(const stringstream& msg)  { Log(msg.str()); }
    void Logger::Log(const wstringstream& msg) { Log(msg.str()); }
    void Logger::Log(const wstring& msg)       { Log(msg.c_str()); }
    void Logger::Log(const wchar_t* msg)       { Log() << QString::fromStdWString(wstring{msg}).toStdString().c_str() << endl; }

    void Logger::LogException(const char* msg)
    {
        LogFile << endl << endl;
        Log("\t\t\t\tI'VE GOT A PRESENT FOR YA");
        Log(msg);
    }
#pragma endregion

#pragma region Support methods
    const string Logger::GetWindowsBit() const
    {
        if (Registry::GetWindowsBit() == Registry::WindowsBit::Win32)
            return "32-bit";
        else
            return "64-bit";
    }
#pragma endregion
