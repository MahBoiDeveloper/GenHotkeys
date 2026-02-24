#pragma once
#include <memory>
#include <QString>
#include "../libs/discord-rpc/include/discord_rpc.h"

class DiscordRPCManager final
{
private: // Data
    QString state = "Editing hotkeys";
    QString details = "";
public: 
    const char* APPLICATION_ID = "830497958034145300";
    const int   SendPresence = 1;

public: // Methods
    DiscordRPCManager();
    ~DiscordRPCManager();
    void Initialize();
    static void HandleDiscordReady(const DiscordUser* connectedUser);
    static void HandleDiscordDisconnected(int errcode, const char* message);
    static void HandleDiscordError(int errcode, const char* message);
    void UpdateDiscordPresence();
};
