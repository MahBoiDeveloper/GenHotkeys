#include <filesystem>
#include "../libs/SteamworksSDK/public/steam/steam_api.h"
#include "Logger.hpp"
#include "ProgramConstants.hpp"
#include "SteamManager.hpp"

using namespace std;
using namespace StringExt;

SteamManager::SteamManager()  {}
SteamManager::~SteamManager() { Dispose(); }

void SteamManager::Initialize()
{
    QString steamapitxt = PROGRAM_CONSTANTS->BINARIES_FOLDER + "\\steam_appid.txt"q;
    appId = QString::number(PROGRAM_CONSTANTS->pSettingsFile->GetSteamAppID());
        
    if (!filesystem::exists(steamapitxt.toStdString().c_str()))
    {
        LOGMSG("steam_appid.txt not found, generating file");
        ofstream file(steamapitxt.toStdString().c_str());
        
        if (!file.is_open())
            file.open(steamapitxt.toStdString().c_str());

        file << appId.toStdString().c_str() << endl;
    }

    LOGMSG("Initializing Steam integration...");
    SteamErrMsg errMsg = { 0 };
    if (SteamAPI_InitEx( &errMsg ) != k_ESteamAPIInitResult_OK)
    {
        LOGSTM << "SteamAPI_Init() failed: " << errMsg << endl;
        LOGMSG("Failed to initialize Steam integration");
    }
    else
    {
        LOGMSG("Steam Integration running");
    }
}

void SteamManager::Dispose()
{
    SteamAPI_Shutdown();
}

QString SteamManager::GetAppID()                  { return appId; }
void    SteamManager::SetAppID(const QString& id) { appId = id; }
