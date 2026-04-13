#include "Logger.hpp"
#include "DiscordManager.hpp"

DiscordManager::DiscordManager()
{
}

DiscordManager::~DiscordManager()
{
    Dispose();
}

void DiscordManager::Initialize()
{
    if (isInitialized)
    {
        UpdateDiscordPresence();
        return;
    }

    LOGMSG("Initializing Discord RPC...");
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready        = &HandleDiscordReady;
    handlers.disconnected = &HandleDiscordDisconnected;
    handlers.errored      = &HandleDiscordError;
    Discord_Initialize(appID, NULL, 1, NULL);
    UpdateDiscordPresence();
    isInitialized = true;
    LOGMSG("Discord RPC is running");
}

void DiscordManager::Dispose()
{
    if (!isInitialized)
        return;

    Discord_Shutdown();
    isInitialized = false;
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
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = state;
    discordPresence.details = details;
    discordPresence.instance = 0;
    Discord_UpdatePresence(&discordPresence);
}
