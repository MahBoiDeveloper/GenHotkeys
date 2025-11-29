// std headers
#include <fcntl.h>   // Allows to use UTF-16 encoding as the default encoding
#include <windows.h> // Allows disable console
#include <iostream>
#include <filesystem>

// Qt headers
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

// Project headers
#include "GUI/WindowManager.hpp"
#include "FactionsManager.hpp"
#include "ProgramConstants.hpp"
#include "Logger.hpp"

using namespace std;

int ShowErrorMessage(const QString& txt)
{
    // Log exception message
    LOGGER->LogException(txt.toUtf8().constData());
    
    // And show it to user
    QMessageBox::critical(nullptr, LOGGER->EXCEPTION_HEADER, txt);

    // Also return -1 as an error
    return -1;
}

int main(int argc, const char** argv)
{
    // After this all out text to console MUST be showed via std::wcout and all chars should be converted as wchar_t
    // _setmode(_fileno(stdout), _O_U16TEXT);

    // Call parsing the Settings.json
    PROGRAM_CONSTANTS = make_unique<ProgramConstants>();

    // Initialize main Qt application
    QApplication HotkeyEditor(argc, const_cast<char**>(argv));

    QString workingDirectory = QString::fromStdWString(filesystem::current_path().c_str());
    
    if (workingDirectory.indexOf(PROGRAM_CONSTANTS->BINARIES_FOLDER) != -1)
    {
        LOGMSG("Program started from Resources\\Binaries folder. Redirect working directory to the ..\\..\\");
        LOGMSG("Before redirect: " + workingDirectory);
        LOGMSG("After redirect:  " + workingDirectory.replace(PROGRAM_CONSTANTS->BINARIES_FOLDER, "", Qt::CaseSensitive));
        filesystem::current_path(workingDirectory.toStdWString());
    }

    if (!filesystem::exists(PROGRAM_CONSTANTS->SETTINGS_FILE.toStdString().c_str()))
        return ShowErrorMessage(PROGRAM_CONSTANTS->SETTINGS_NO_FOUND);

    if (!filesystem::exists(PROGRAM_CONSTANTS->TECH_TREE_FILE.toStdString().c_str()))
        return ShowErrorMessage(PROGRAM_CONSTANTS->TECH_TREE_NO_FOUND);

    if (!filesystem::exists(PROGRAM_CONSTANTS->ICONS_FOLDER.toStdString().c_str()))
        return ShowErrorMessage(PROGRAM_CONSTANTS->ICONS_FOLDER_NO_FOUND);

    if (!filesystem::exists(PROGRAM_CONSTANTS->THEME_FOLDER.toStdString().c_str()))
        return ShowErrorMessage(PROGRAM_CONSTANTS->THEME_FOLDER_NO_FOUND);

    if (!filesystem::exists(PROGRAM_CONSTANTS->TRANSLATIONS_FOLDER.toStdString().c_str()))
        return ShowErrorMessage(PROGRAM_CONSTANTS->TRANSLATIONS_NO_FOUND);

    PROGRAM_CONSTANTS->InitializeTranslations();
    PROGRAM_CONSTANTS->InitializeFileSettings();

    // Show console, that by default is hiding by Logger class
    if (PROGRAM_CONSTANTS->pSettingsFile->IsConsoleEnabled()) 
        ShowWindow(GetConsoleWindow(), SW_SHOW);

    // Initialize TechTree.json parsing
    FACTIONS_MANAGER = make_unique<FactionManager>();

    // Define logger as the singleton class, that could be used anywhere in the project
    WINDOW_MANAGER = make_unique<WindowManager>();

    try
    {
        WINDOW_MANAGER->Show();
        HotkeyEditor.exec();
    }
    catch (const exception& exception) { return ShowErrorMessage(exception.what()); }
    catch (const char* msg)            { return ShowErrorMessage(msg); }
    catch (const string& msg)          { return ShowErrorMessage(msg.c_str()); }
    catch (const QString& msg)         { return ShowErrorMessage(msg.toStdString().c_str()); }
    catch (...)                        { return ShowErrorMessage(PROGRAM_CONSTANTS->UNKNOWN_ERROR); }

    return 0;
}
