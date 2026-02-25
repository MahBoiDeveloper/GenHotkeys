#include "Logger.hpp"
#include "DiscordManager.hpp"

DiscordManager::DiscordManager()
{
}

DiscordManager::~DiscordManager()
{
    Discord_Shutdown();
}

void DiscordManager::Initialize()
{
    LOGMSG("Initializing Discord RPC...");
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready        = &HandleDiscordReady;
    handlers.disconnected = &HandleDiscordDisconnected;
    handlers.errored      = &HandleDiscordError;
    Discord_Initialize(AppID, NULL, 1, NULL);
    UpdateDiscordPresence();
    LOGMSG("Discord RPC is running");
}

void DiscordManager::Dispose()
{
    Discord_Shutdown();
}

void DiscordManager::HandleDiscordReady(const DiscordUser* connectedUser)
{
    LOGMSG("Discord: connected to user" + connectedUser->username + "#" + connectedUser->discriminator + " - " + connectedUser->userId);
}

void DiscordManager::HandleDiscordDisconnected(int errcode, const char* message)
{
    LOGMSG("Discord: disconnected (" + errcode + ":" + message);
}

void DiscordManager::HandleDiscordError(int errcode, const char* message)
{
    LOGMSG("Discord: error (" + errcode + ":" + message);
}

void DiscordManager::UpdateDiscordPresence()
{
    if (SendPresence)
    {
        char buffer[256];
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));
        discordPresence.state = state.toStdString().c_str();
        discordPresence.details = details.toStdString().c_str();
        discordPresence.instance = 0;
        Discord_UpdatePresence(&discordPresence);
    }
    else
    {
        Discord_ClearPresence();
    }
}
