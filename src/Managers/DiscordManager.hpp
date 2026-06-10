#pragma once
#include <memory>
#include <QString>
#include "../libs/discord-rpc/include/discord_rpc.h"

#define DISCORD_MANAGER DiscordManager::Instance

class DiscordManager final
{
private: // Data
    const char* appID = "1476177039966081097";
    const char* state = "Editing hotkeys";
    const char* details = "";
public:
    inline const static std::unique_ptr<DiscordManager> Instance = std::make_unique<DiscordManager>();

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
