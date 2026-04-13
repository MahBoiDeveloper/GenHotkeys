#pragma once
#include <string>
#include <map>
#include <vector>
#include <QString>

namespace Windows
{
    class Registry final
    {
    public: // Data
        enum class Arch
        {
            Win32,
            Win64
        };

        enum class Games
        {
            Generals,
            GeneralsZeroHour
        };

        enum class RootFolder
        {
            HKLM,
            HKCU
        };

        /// @brief Candidate registry keys for installed C&C: Generals and Zero Hour.
        inline static const std::map<Games, std::vector<std::wstring>> PATHS_TO_GAMES =
        {
            {Games::Generals,
                {
                    L"SOFTWARE\\Electronic Arts\\EA Games\\Generals",
                    L"SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\Generals"
                }},
            {Games::GeneralsZeroHour,
                {
                    L"SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour",
                    L"SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour",
                    L"SOFTWARE\\Electronic Arts\\EA Games\\ZeroHour",
                    L"SOFTWARE\\WOW6432Node\\Electronic Arts\\EA Games\\ZeroHour"
                }},
        };

    public: // Methods
        Registry() = delete;
        
        /// @brief Returns equal string for enum class value.
        static std::string ToString(Games game);
        /// @brief Returns equal string for enum class value. 
        static QString ToQString(Games game);
        /// @brief Returns actual Windows bit like a enum value.
        static Arch GetWindowsBit();
        
        /// @brief Returns REG_SZ string value from MS Windows registry.
        static std::string GetTextFromKeyA(RootFolder Folder, const char* pPathToFolder, const char* pKeyName);
        /// @brief Returns REG_SZ string value from MS Windows registry.
        static std::wstring GetTextFromKeyW(RootFolder Folder, const wchar_t* pPathToFolder, const wchar_t* pKeyName);

        /// @brief Checks if Windows is 64-bit.
        static bool IsWindow64bit();
        /// @brief Checks if Windows is 32-bit.
        static bool IsWindow32bit();

        /// @brief Sets paths to all games (C&C: Generals and C&C: Generals - Zero Hour).
        static std::wstring GetPathToGame(Games game);
        /// @brief Returns current user language from HKCU\\Control Panel\\International\\Geo\\Name (example: EN).
        static std::string GetCurrentUserLanguage();
        /// @brief Returns Windows version from HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProductName.
        static std::string GetWindowsVersion();
        /// @brief Returns processor vendor infomation from HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\ProcessorNameString.
        static std::string GetProcessorInfo();
        /// @brief Returns current all memory size information in Mebibytes.
        static std::string GetMemoryInfo();
        /// @brief Returns universal unique identifier as a string.
        static std::string GetUUID();
    };
}
