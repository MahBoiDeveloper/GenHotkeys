#pragma comment(lib, "rpcrt4.lib")
#include <filesystem>
#include <sstream>
#include <windows.h>
#include <tchar.h>

#include "../Extensions/StringExt.hpp"
#include "Registry.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace
{
    HKEY ToNativeRootKey(const Windows::Registry::RootFolder folder)
    {
        switch (folder)
        {
            case Windows::Registry::RootFolder::HKCU: return HKEY_CURRENT_USER;
            case Windows::Registry::RootFolder::HKLM:
            default:                                  return HKEY_LOCAL_MACHINE;
        }
    }

    template<typename TString>
    TString TrimTrailingNulls(TString value)
    {
        while (!value.empty() && value.back() == 0)
            value.pop_back();

        return value;
    }

    wstring TrimTrailingSlashes(wstring value)
    {
        while (!value.empty() && (value.back() == L'\\' || value.back() == L'/'))
            value.pop_back();

        return value;
    }

    string QueryRegistryStringA(HKEY root, const char* path, const char* keyName, const REGSAM samDesired)
    {
        HKEY rKey = nullptr;
        if (RegOpenKeyExA(root, path, 0, samDesired, &rKey) != ERROR_SUCCESS)
            return {};

        DWORD type = 0;
        DWORD size = 0;
        if (RegQueryValueExA(rKey, keyName, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
            (type != REG_SZ && type != REG_EXPAND_SZ))
        {
            RegCloseKey(rKey);
            return {};
        }

        string value(size, '\0');
        if (RegQueryValueExA(rKey, keyName, nullptr, nullptr, reinterpret_cast<LPBYTE>(value.data()), &size) != ERROR_SUCCESS)
        {
            RegCloseKey(rKey);
            return {};
        }

        RegCloseKey(rKey);
        return TrimTrailingNulls(std::move(value));
    }

    wstring QueryRegistryStringW(HKEY root, const wchar_t* path, const wchar_t* keyName, const REGSAM samDesired)
    {
        HKEY rKey = nullptr;
        if (RegOpenKeyExW(root, path, 0, samDesired, &rKey) != ERROR_SUCCESS)
            return {};

        DWORD type = 0;
        DWORD size = 0;
        if (RegQueryValueExW(rKey, keyName, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
            (type != REG_SZ && type != REG_EXPAND_SZ))
        {
            RegCloseKey(rKey);
            return {};
        }

        wstring value(size / sizeof(wchar_t), L'\0');
        if (RegQueryValueExW(rKey, keyName, nullptr, nullptr, reinterpret_cast<LPBYTE>(value.data()), &size) != ERROR_SUCCESS)
        {
            RegCloseKey(rKey);
            return {};
        }

        RegCloseKey(rKey);
        return TrimTrailingNulls(std::move(value));
    }

    wstring FindInstalledGamePath(const Windows::Registry::Games game)
    {
        static const vector<wstring> valueNames =
        {
            L"InstallPath",
            L"Install Dir",
            L"InstallDir"
        };

        static const vector<Windows::Registry::RootFolder> roots =
        {
            Windows::Registry::RootFolder::HKLM,
            Windows::Registry::RootFolder::HKCU
        };

        static const vector<REGSAM> views =
        {
            KEY_READ | KEY_WOW64_32KEY,
            KEY_READ | KEY_WOW64_64KEY,
            KEY_READ
        };

        const auto pathsIt = Windows::Registry::PATHS_TO_GAMES.find(game);
        if (pathsIt == Windows::Registry::PATHS_TO_GAMES.end())
            return {};

        for (const auto root : roots)
        {
            const HKEY nativeRoot = ToNativeRootKey(root);

            for (const auto& path : pathsIt->second)
            {
                for (const auto view : views)
                {
                    for (const auto& valueName : valueNames)
                    {
                        wstring result = QueryRegistryStringW(nativeRoot, path.c_str(), valueName.c_str(), view);
                        if (!result.empty())
                            return TrimTrailingSlashes(std::move(result));
                    }
                }
            }
        }

        return {};
    }

    wstring DeriveGeneralsPathFromZeroHourInstall()
    {
        const wstring zeroHourPath = FindInstalledGamePath(Windows::Registry::Games::GeneralsZeroHour);
        if (zeroHourPath.empty())
            return {};

        const fs::path rootPath = fs::path(zeroHourPath);
        if (fs::exists(rootPath / "Data" / "English" / "generals.csf"))
            return rootPath.wstring();

        const fs::path bundledGeneralsPath = rootPath / "ZH_Generals";
        if (fs::exists(bundledGeneralsPath / "Data" / "English" / "generals.csf"))
            return bundledGeneralsPath.wstring();

        return {};
    }
}

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
        return QueryRegistryStringA(ToNativeRootKey(Folder), pPathToFolder, pKeyName, KEY_READ);
    }

    wstring Registry::GetTextFromKeyW(const Registry::RootFolder Folder, const wchar_t* pPathToFolder, const wchar_t* pKeyName)
    {
        return QueryRegistryStringW(ToNativeRootKey(Folder), pPathToFolder, pKeyName, KEY_READ);
    }

    wstring Registry::GetPathToGame(const Games game)
    {
        wstring directPath = FindInstalledGamePath(game);
        if (!directPath.empty())
            return directPath;

        if (game == Games::Generals)
            return DeriveGeneralsPathFromZeroHourInstall();

        return {};
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
