#pragma comment(lib, "rpcrt4.lib")
#include <sstream>
#include <windows.h>
#include <tchar.h>

#include "../StringExt.hpp"
#include "Registry.hpp"

using namespace std;

namespace Windows
{
    Registry::Arch Registry::GetWindowsBit()
    {
        HKEY rKey;
        Arch windowsBit;

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\WOW6432Node"), 0, KEY_QUERY_VALUE, &rKey) == ERROR_SUCCESS)
            windowsBit = Arch::Win64;
        else
            windowsBit = Arch::Win32;

        RegCloseKey(rKey);

        return windowsBit;
    }

    string Registry::GetTextFromKeyA(const Registry::RootFolder Folder, const char* pPathToFolder, const char* pKeyName)
    {
        HKEY  rKey;
        DWORD Size = 256;
        TCHAR Reget[Size] = { 0 };

        switch (Folder)
        {
            case Registry::RootFolder::HKCU :
                RegOpenKeyExA(HKEY_CURRENT_USER, pPathToFolder, 0, KEY_READ, &rKey);
                break;
            
            case Registry::RootFolder::HKLM :
                RegOpenKeyExA(HKEY_LOCAL_MACHINE, pPathToFolder, 0, KEY_READ, &rKey);
                break;
        }

        RegQueryValueExA(rKey, pKeyName, NULL, NULL, (LPBYTE)&Reget, &Size);
        RegCloseKey(rKey);
        
        string returnValue(Reget);
        returnValue.shrink_to_fit();
            
        return returnValue;
    }

    wstring Registry::GetTextFromKeyW(const Registry::RootFolder Folder, const wchar_t* pPathToFolder, const wchar_t* pKeyName)
    {
        HKEY  rKey;
        DWORD Size = 256;
        WCHAR Reget[Size] = { 0 };

        switch (Folder)
        {
            case Registry::RootFolder::HKCU :
                RegOpenKeyExW(HKEY_CURRENT_USER, pPathToFolder, 0, KEY_READ, &rKey);
                break;
            
            case Registry::RootFolder::HKLM :
                RegOpenKeyExW(HKEY_LOCAL_MACHINE, pPathToFolder, 0, KEY_READ, &rKey);
                break;
        }

        RegQueryValueExW(rKey, pKeyName, NULL, NULL, (LPBYTE)&Reget, &Size);
        RegCloseKey(rKey);
        
        wstring returnValue(Reget);
        returnValue.shrink_to_fit();
            
        return returnValue;
    }

    wstring Registry::GetPathToGame(const Games game)
    {
        wstring Key  = L"InstallPath";
        wstring Path = Registry::PATHS_TO_GAMES.find(game)->second.find(GetWindowsBit())->second;
        return GetTextFromKeyW(Registry::RootFolder::HKLM, Path.c_str(), Key.c_str());
    }

    string Registry::ToString(Games game)
    {
        string returnValue;

        switch (game)
        {
            case Games::Generals:
                returnValue = "Generals";
                break;
            case Games::GeneralsZeroHour:
                returnValue = "Generals Zero Hour";
                break;
        }

        return returnValue;
    }

    QString Registry::ToQString(Games game) { return QString::fromStdString(Registry::ToString(game)); }

    bool Registry::IsWindow64bit() { return GetWindowsBit() == Arch::Win64; }
    bool Registry::IsWindow32bit() { return GetWindowsBit() == Arch::Win32; }

#pragma region Logger methods
        string Registry::GetCurrentUserLanguage()
        {
            const char Path[] = {"Control Panel\\International"};
            const char Key[]  = {"LocaleName"};
            return GetTextFromKeyA(Registry::RootFolder::HKCU, &Path[0], &Key[0]).substr(0, 2);
        }

        string Registry::GetWindowsVersion()
        {
            const char Path[] = {"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"};
            const char Key[]  = {"ProductName"};
            return GetTextFromKeyA(Registry::RootFolder::HKLM, &Path[0], &Key[0]);
        }

        string Registry::GetProcessorInfo()
        {
            const char Path[]  = {"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"};
            const char Value[] = {"ProcessorNameString"};
            return GetTextFromKeyA(Registry::RootFolder::HKLM, &Path[0], &Value[0]);
        }

        string Registry::GetMemoryInfo()
        {
            stringstream ss;
            MEMORYSTATUSEX MemStat;
            MemStat.dwLength = sizeof (MemStat);
            GlobalMemoryStatusEx(&MemStat);

            ss << (MemStat.ullTotalPhys/1024)/1024 << "MiB";
            return ss.str();
        }

        string Registry::GetUUID()
        {
            // Magic code by stackoverflow: https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
            stringstream ss;

            UUID uuid;
            auto tmpUuidCreate = UuidCreate(&uuid);
            char* str;
            auto tmpUuidToStringA = UuidToStringA(&uuid, reinterpret_cast<RPC_CSTR*>(&str));
            ss << str;
            RpcStringFreeA(reinterpret_cast<RPC_CSTR*>(&str));

            return ss.str();
        }
#pragma endregion
}
