#pragma once
#include <memory>
#include <QString>
#include "../libs/discord-rpc/include/discord_rpc.h"

#define DISCORD_MANAGER DiscordManager::Instance

class DiscordManager final
{
private: // Data
    QString state = "Editing hotkeys";
    QString details = "";
public:
    inline static std::unique_ptr<DiscordManager> Instance = std::make_unique<DiscordManager>();
    const char* AppID = "830497958034145300";
    const int   SendPresence = 1;

private: // Methods
    static void HandleDiscordReady(const DiscordUser* connectedUser);
    static void HandleDiscordDisconnected(int errcode, const char* message);
    static void HandleDiscordError(int errcode, const char* message);
public:
    DiscordManager();
    ~DiscordManager();
    void Initialize();
    void Dispose();
    void UpdateDiscordPresence();
};
