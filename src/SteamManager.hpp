#pragma once
#include <memory>
#include <QString>

#define STEAM_MANAGER SteamManager::Instance

class SteamManager final
{
private: // Data
    QString appId;
public:
    inline static std::unique_ptr<SteamManager> Instance = std::make_unique<SteamManager>();

private: // Methods
public:
    SteamManager();
    ~SteamManager();

    void Initialize();
    void Dispose();

    void SetAppID(const QString& id);
    QString GetAppID();
};
