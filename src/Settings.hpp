#pragma once
#include <QSet>
#include "Extensions/BoolExt.hpp"
#include "Parsers/JSONFile.hpp"

#define SET_PROPERTY(x, y) if constexpr(std::same_as<T, bool>) { x = y; } else { x = FromQtCheckState(y); }

class SettingsWindow;

class Settings final
{
private: // Data
    JSONFile      json;
    QSet<Qt::Key> AllowedHotkeys;
    bool          DebugConsole;
    bool          DiscordRPC;
    bool          ForceSystemLanguage;
    size_t        Language;
    bool          StatusBar;
    bool          SteamAPI;

private: // Methods
public:
    explicit Settings();
    ~Settings();

    /// @brief Set all `Settings` class variables to default values.
    void SetToDefault();
    /// @brief Parse `Resources\Settings.json`.
    void Parse();
    /// @brief Save changes to the `Resources\Settings.json`.
    void Save();

    /// @brief Returns `QSet` of available keys from `QWEWRTY` keyboard to choice by user.
    const QSet<Qt::Key> GetAllowedKeys() const;
    /// @brief Returns value of current language from settings file corresponded to the hashtable from the `ProgramConstants`.
    const size_t GetLanguage() const;
    /// @brief Returns status for console from settings file.
    const bool IsConsoleEnabled() const;
    /// @brief Returns status for Discord RPC from settings file.
    const bool IsDiscordRPCEnabled() const;
    /// @brief Returns status for force editor use system language from settings file.
    const bool IsForceSystemLanguageOnStartUpEnabled() const;
    /// @brief Returns state of the status bar from settings file.
    const bool IsStatusBarEnabled() const;
    /// @brief Returns status for Steam API from settings file
    const bool IsSteamIntegrationEnabled() const;

    /// @brief Sets `QSet` of allowed keys.
    void SetAllowedKeys(const QSet<Qt::Key>& keys);
    /// @brief Sets editor's language.
    void SetLanguage(const size_t& locale);
    /// @brief Sets status for console window.
    template<BoolExt::IsStateDescriptor T> void SetConsoleStatus(const T state)                { DebugConsole        = BoolExt::ToBool(state); }
    /// @brief Sets status for Discord RPC.
    template<BoolExt::IsStateDescriptor T> void SetDiscordRPCStatus(const T state)             { DiscordRPC          = BoolExt::ToBool(state); }
    /// @brief Sets status to force editor use system language on the start.
    template<BoolExt::IsStateDescriptor T> void SetForceSystemLanguageOnStartUp(const T state) { ForceSystemLanguage = BoolExt::ToBool(state); }
    /// @brief Sets status for console window.
    template<BoolExt::IsStateDescriptor T> void SetStatusBarStatus(const T state)              { StatusBar           = BoolExt::ToBool(state); }
    /// @brief Sets status for Steam API Integration.
    template<BoolExt::IsStateDescriptor T> void SetSteamIntegrationStatus(const T state)       { SteamAPI            = BoolExt::ToBool(state); }
};
