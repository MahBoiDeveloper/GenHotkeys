#ifdef PROJECT_EXE_RELATIVE_PATH

#define EXCEPTION_HEADER "I'VE GOT A PRESENT FOR YA"
#define RESOURCES        L"Resources"
#define BINARIES         L"Resources/Binaries"
#define ERROR_HEADER     L"Error"

#include <string>
#include <filesystem>
#include <iostream>
#include <windows.h>
#include <shellapi.h>

// Links local executable to the main application of the project
int main(int argc, const char** argv)
{
    // Disable console from the start
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Resolve all paths relative to the starter, preserving Unicode characters.
    std::wstring modulePath(MAX_PATH, L'\0');
    for (;;)
    {
        const DWORD length = GetModuleFileNameW(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));

        if (length == 0)
        {
            const std::wstring msg = L"Unable to locate starter, error code: " + std::to_wstring(GetLastError());
            std::wcout << msg << std::endl;
            MessageBoxW(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
            return -1;
        }

        if (length < modulePath.size())
        {
            modulePath.resize(length);
            break;
        }

        modulePath.resize(modulePath.size() * 2);
    }
    const std::filesystem::path starterDir = std::filesystem::path(modulePath).parent_path();

    // Create /Logs if they doesn't exists
    std::filesystem::create_directory(starterDir / L"Logs");

    if (!std::filesystem::exists(starterDir / RESOURCES))
    {
        std::wstring msg = std::wstring(RESOURCES) + L" folder doesn't exist!" + L'\n' +
                           L"You cannot use Hotkey Editor without configs and Qt binary files." + L'\n' +
                           L"Please, check your installation instruction." + L'\n';
        std::wcout << msg << std::endl;
        MessageBoxW(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    if (!std::filesystem::exists(starterDir / BINARIES))
    {
        std::wstring msg = std::wstring(BINARIES) + L" folder doesn't exist!" + L'\n'
                         + L"You cannot use Hotkey Editor without configs and Qt binary files." + L'\n'
                         + L"Please, check your installation instruction." + L'\n';
        std::wcout << msg << std::endl;
        MessageBoxW(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    const std::filesystem::path exePath = (starterDir / std::filesystem::u8path(PROJECT_EXE_RELATIVE_PATH)).lexically_normal();
    const std::filesystem::path dir     = exePath.parent_path();

    HINSTANCE hRet = ShellExecuteW(
        HWND_DESKTOP,    // Parent window
        L"open",         // Operation to perform
        exePath.c_str(), // Absolute path to program
        nullptr,         // Parameters
        dir.c_str(),     // Default directory
        SW_HIDE);        // How to open

    if (reinterpret_cast<size_t>(hRet) <= 32)
    {
        std::wstring msg = L"Unable to start editor, error code: " + std::to_wstring(reinterpret_cast<INT_PTR>(hRet));
        MessageBoxW(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    return 0;
}
#endif
