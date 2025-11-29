#include <QSet>
#include "Windows/Locale.hpp"
#include "Parsers/CSFParser.hpp"
#include "Parsers/JSONFile.hpp"
#include "Logger.hpp"
#include "Faction.hpp"

Faction::Faction(const QString& _shortName, const QString& _displayName, const QString& _displayNameDescription)
    : shortName{_shortName}
    , displayName{_displayName}
    , displayNameDescription{_displayNameDescription}
{
}

Faction::Faction(const QJsonObject& factionAsObject)
    : shortName{factionAsObject[PROGRAM_CONSTANTS->SHORT_NAME].toString()}
    , displayName{factionAsObject[PROGRAM_CONSTANTS->DISPLAY_NAME].toString()}
    , displayNameDescription{factionAsObject[PROGRAM_CONSTANTS->DISPLAY_NAME_DESCRIPTION].toString()}
    , techTree{ParseJsonObject(factionAsObject)}
{
    for (size_t lng : PROGRAM_CONSTANTS->Languages.keys())
    {
        QString _displayName = "";
        QString _displayNameDescription = "";
        
        if (lng != PROGRAM_CONSTANTS->DEFAULT_LANGUAGE_CODE)
        {
            auto translatedNames = factionAsObject[PROGRAM_CONSTANTS->Languages.value(lng).first].toObject();
            
            if (!translatedNames.isEmpty())
            {
                if (!translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME].isNull() 
                    && !translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME].isUndefined())
                {
                    _displayName = translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME].toString();
                }

                if (!translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME_DESCRIPTION].isNull() 
                    && !translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME_DESCRIPTION].isUndefined())
                {
                    _displayNameDescription = translatedNames[PROGRAM_CONSTANTS->DISPLAY_NAME_DESCRIPTION].toString();
                }
            }
        }
        else
        {
            continue;
        }

        localizedDisplay.insert(lng, {_displayName, _displayNameDescription});
    }
}

const QString Faction::GetShortName() const { return shortName; }
const QString Faction::GetDisplayName() const { return displayName; }
const QString Faction::GetDisplayNameDescription() const { return displayNameDescription; }
const QMap<Faction::GameObject, GameObjectTypes>& Faction::GetTechTree() const { return techTree; }

const QString Faction::GetDisplayName(size_t lng) const
{
    QString ret;

    if (lng != PROGRAM_CONSTANTS->DEFAULT_LANGUAGE_CODE)
        ret = localizedDisplay.value(lng).first;
    
    if (ret == StringExt::EmptyString)
        ret = displayName;

    return ret;
}
const QString Faction::GetDisplayNameDescription(size_t lng) const 
{
    QString ret;

    if (lng != PROGRAM_CONSTANTS->DEFAULT_LANGUAGE_CODE)
        ret = localizedDisplay.value(lng).second;
    
    if (ret == StringExt::EmptyString)
        ret = displayNameDescription;

    return ret;
}

const QVector<QVector<Faction::Action>> Faction::GetKeyboardLayoutsByObjectName(const QString& objName) const
{
    for(const Faction::GameObject& go : techTree.keys())
        if(go.iconName == objName)
            return go.keyboardLayouts;

    return {};
}

QMap<Faction::GameObject, GameObjectTypes> Faction::ParseJsonObject(const QJsonObject& obj)
{
    QMap<GameObject, GameObjectTypes> tmpMap;

    // Circle for each element in {"Buildings", "Infantry", "Vehicles", "Aircrafts"} map
    for(const QString& qstrObjectsArray : PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS)
    {
        QJsonArray currArr  = obj[qstrObjectsArray].toArray();

        for(const auto& elemGameObj : currArr)
        {
            QJsonObject currGameObj      = elemGameObj.toObject();
            QString     _name            = currGameObj["Name"].toString();
            QString     _ingameName      = currGameObj["IngameName"].toString();
            QJsonArray  _keyboardLayouts = currGameObj["KeyboardLayouts"].toArray();

            QVector<QVector<Action>> _layouts;

            for(const auto& elemLayout : _keyboardLayouts)
            {
                QJsonArray currLayout = elemLayout.toArray();
                QVector<Action> _layout;    

                for(const auto& act : currLayout)
                {
                    QJsonObject currAct = act.toObject();
                    _layout.push_back(Action{currAct["IconName"].toString(), currAct["HotkeyString"].toString()});
                }

                _layouts.push_back(_layout);
            }

            tmpMap.insert(Faction::GameObject{_name, _ingameName, _layouts}, PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS.key(qstrObjectsArray));
        }
    }
    
    return tmpMap;
}

void Faction::SetHotkey(const QString& goName, const QString& actName, const QString& hk)
{
    for(Faction::GameObject& currGO : techTree.keys())
    {
        if(currGO.iconName != goName)
            continue;

        for(QVector<Faction::Action> currLt : currGO.keyboardLayouts)
        {
            for(Faction::Action currAct : currLt)
            {
                if(currAct.iconName != actName)
                    continue;
                
                CSF_PARSER->SetHotkey(currAct.hotkeyString, hk.toStdWString()[0]);
                break;
            }
        }
        break;
    }
}

// Method used for QMap.insert
bool operator < (Faction::GameObject a, Faction::GameObject b) { return a.iconName < b.iconName ? true : false; }
