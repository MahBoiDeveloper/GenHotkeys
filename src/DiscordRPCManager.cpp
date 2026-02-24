#include "Logger.hpp"
#include "DiscordRPCManager.hpp"

DiscordRPCManager::DiscordRPCManager()
{
}

DiscordRPCManager::~DiscordRPCManager()
{
    Discord_Shutdown();
}

void DiscordRPCManager::Initialize()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready        = &HandleDiscordReady;
    handlers.disconnected = &HandleDiscordDisconnected;
    handlers.errored      = &HandleDiscordError;
    Discord_Initialize(APPLICATION_ID, NULL, 1, NULL);
    UpdateDiscordPresence();
}

void DiscordRPCManager::HandleDiscordReady(const DiscordUser* connectedUser)
{
    LOGMSG("Discord: connected to user" + connectedUser->username + "#" + connectedUser->discriminator + " - " + connectedUser->userId);
}

void DiscordRPCManager::HandleDiscordDisconnected(int errcode, const char* message)
{
    LOGMSG("Discord: disconnected (" + errcode + ":" + message);
}

void DiscordRPCManager::HandleDiscordError(int errcode, const char* message)
{
    LOGMSG("Discord: error (" + errcode + ":" + message);
}

void DiscordRPCManager::UpdateDiscordPresence()
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
