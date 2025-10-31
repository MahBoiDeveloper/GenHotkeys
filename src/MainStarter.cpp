#ifdef PROJECT_EXE_RELATIVE_PATH

#define EXCEPTION_HEADER "I'VE GOT A PRESENT FOR YA"
#define RESOURCES "Resources"
#define BINARIES "Resources/Binaries"
#define ERROR_HEADER "Error"

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <windows.h>

// Links local executable to the main application of the project
int main(int argc, const char** argv)
{
    // Disable console from the start
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Create /Logs if they doesn't exists
    std::filesystem::create_directory("Logs");

    if (!std::filesystem::exists(RESOURCES))
    {
        std::string msg = std::string() + RESOURCES + " folder doesn't exist!" + '\n' +
                          "You cannot use Hotkey Editor without configs and Qt binary files." + '\n' +
                          "Please, check your installation instruction." + '\n';
        std::cout << msg.c_str() << std::endl;
        MessageBox(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    if (!std::filesystem::exists(BINARIES))
    {
        std::string msg = std::string() + BINARIES + " folder doesn't exist!" + '\n' +
                          "You cannot use Hotkey Editor without configs and Qt binary files." + '\n' +
                          "Please, check your installation instruction." + '\n';
        std::cout << msg.c_str() << std::endl;
        MessageBox(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    std::stringstream ss(PROJECT_EXE_RELATIVE_PATH);
    std::string tmp = "";
    const char del = '/';
    std::vector<std::string> vec;
    vec.reserve(3);

    // Split Cmake's path by / in PROJECT_EXE_RELATIVE_PATH string
    while (getline(ss, tmp, del))
        vec.push_back(tmp);

    // Combine all dirs into single string
    std::string dir = vec.at(0);
    for (int i = 1; i < vec.size() - 1; i++)
        dir += '\\' + vec.at(i);

    // Refactor later to work with UTF-8 and provide args to the real executable.
    HINSTANCE hRet = ShellExecuteA(
        HWND_DESKTOP,                   // Parent window
        "open",                         // Operation to perform
        vec.at(vec.size() - 1).c_str(), // Path to program
        NULL,                           // Parameters
        dir.c_str(),                    // Default directory
        SW_HIDE);                       // How to open

    if (reinterpret_cast<size_t>(hRet) <= 32)
    {
        std::string msg = std::string() + "Unable to start editor, error code: " + std::to_string(reinterpret_cast<size_t>(hRet));
        MessageBox(HWND_DESKTOP, msg.c_str(), ERROR_HEADER, MB_OK);
        return -1;
    }

    return 0;
}
#endif
