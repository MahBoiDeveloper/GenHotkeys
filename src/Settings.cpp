#include <QFile>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>

#include "Windows/Registry.hpp"
#include "Windows/Locale.hpp"
#include "Convert.hpp"
#include "Logger.hpp"
#include "Settings.hpp"

Settings::Settings()
{
    json = JSONFile(PROGRAM_CONSTANTS->SETTINGS_FILE);
    SetToDefault();
    Parse();
}

Settings::~Settings() 
{
}

void Settings::SetToDefault()
{
    AllowedHotkeys      = PROGRAM_CONSTANTS->DEFAULT_ALLOWED_KEYS;
    DebugConsole        = false;
    DiscordRPC          = true;
    ForceSystemLanguage = true;
    StatusBar           = true;
    SteamAPI            = true;
    SteamAppID          = 2732960; // ZH APP ID
    Language            = 0;       // English
}

void Settings::Parse()
{
    DebugConsole        = json.Query("$." + nameof(DebugConsole)).toBool();
    DiscordRPC          = json.Query("$." + nameof(DiscordRPC)).toBool();
    ForceSystemLanguage = json.Query("$." + nameof(ForceSystemLanguage)).toBool();
    StatusBar           = json.Query("$." + nameof(StatusBar)).toBool();
    SteamAppID          = json.Query("$." + nameof(SteamAppID)).toInt() != 0 ? json.Query("$." + nameof(SteamAppID)).toInt() : SteamAppID;
    SteamAPI            = json.Query("$." + nameof(SteamAPI)).toBool();

    QString twoLetters = StringExt::EmptyString;
    if (ForceSystemLanguage)
        twoLetters = QString::fromStdString(Windows::Registry::GetCurrentUserLanguage()).toLower();
    else
        twoLetters = json.Query("$." + nameof(Language)).toString().toLower();
    
    Language = PROGRAM_CONSTANTS->Languages.key({twoLetters, Windows::Locale::LanguageName(twoLetters)});

    for (const QJsonValue& ch : json.Query("$." + nameof(AllowedHotkeys)).toArray())
        AllowedHotkeys.insert(PROGRAM_CONSTANTS->KEYBOARD_KEYS.value(ch.toString()[0]));
}

void Settings::Save()
{
    QJsonObject jsMainObj                  = json.GetMainObject();
    jsMainObj[nameof(DebugConsole)]        = DebugConsole;
    jsMainObj[nameof(DiscordRPC)]          = DiscordRPC;
    jsMainObj[nameof(ForceSystemLanguage)] = ForceSystemLanguage;
    jsMainObj[nameof(Language)]            = PROGRAM_CONSTANTS->Languages.value(Language).first;
    jsMainObj[nameof(StatusBar)]           = StatusBar;
    jsMainObj[nameof(SteamAppID)]          = StringExt::ToQString(SteamAppID);
    jsMainObj[nameof(SteamAPI)]            = SteamAPI;

    QJsonDocument jsDoc;
    jsDoc.setObject(jsMainObj);

    LOGMSG("Saving changes to the \"" + PROGRAM_CONSTANTS->SETTINGS_FILE + "\"..." );
    QFile settingsJson(PROGRAM_CONSTANTS->SETTINGS_FILE);
    settingsJson.remove();
    settingsJson.open(QIODevice::WriteOnly | QIODevice::Text);
    settingsJson.write(jsDoc.toJson());
    settingsJson.close();
    LOGMSG("Setting changes has been saved");
}

const QSet<Qt::Key> Settings::GetAllowedKeys()                        const { return AllowedHotkeys; }
const size_t        Settings::GetLanguage()                           const { return Language; }
const size_t        Settings::GetSteamAppID()                         const { return SteamAppID; }
const bool          Settings::IsConsoleEnabled()                      const { return DebugConsole; }
const bool          Settings::IsDiscordRPCEnabled()                   const { return DiscordRPC; }
const bool          Settings::IsForceSystemLanguageOnStartUpEnabled() const { return ForceSystemLanguage; }
const bool          Settings::IsStatusBarEnabled()                    const { return StatusBar; }
const bool          Settings::IsSteamIntegrationEnabled()             const { return SteamAPI; }

void Settings::SetAllowedKeys(const QSet<Qt::Key>& keys){ AllowedHotkeys = keys; }
void Settings::SetLanguage(const size_t& code)          { Language       = code; }
