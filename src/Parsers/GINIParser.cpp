#include <QStringList>

#include "../Extensions/ExceptionExt.hpp"
#include "../Core/Logger.hpp"
#include "GINIParser.hpp"

using namespace std;
using namespace StringExt;

#pragma region CTORs and DTORs
    GINIParser::GINIParser(const string& filePath)  : Path(filePath)               { Parse(); }
    GINIParser::GINIParser(const char*   filePath)  : Path(string(filePath))       { Parse(); }
    GINIParser::GINIParser(const QString& filePath) : Path(filePath.toStdString()) { Parse(); }
#pragma endregion

#pragma region Parsing
    void GINIParser::Parse()
    {
        ifstream file(Path, ios::in);

        LOGMSG("Attempt to read file \"" + Path + "\"...");

        if (file.is_open())
        {
            string buff, buffSectionName, buffKeyName, buffKeyValue;
            QString qstr;
            vector<GINIKey> buffKeys;
            uint32_t fileLineIndex = 0;
            size_t sectionIndex = 0;

            while(getline(file, buff))
            {
                fileLineIndex++;
                
                // QString has much more useful features instead of std::string shit
                qstr = QString::fromStdString(buff);

                // Removing all comments and also spaces and tabs
                int commentIndex = qstr.indexOf(';');
                if (commentIndex != -1)
                    qstr = qstr.remove(commentIndex, qstr.size());

                qstr = qstr.trimmed();

                // If string after trimming is empty, then skip this string.
                if (qstr.isEmpty()) continue;

                // The game uses spaces, tabs and '=' as token separators.
                int keyEndIndex = 0;
                while (keyEndIndex < qstr.size() && qstr[keyEndIndex] != ' ' &&
                       qstr[keyEndIndex] != '\t' && qstr[keyEndIndex] != '=')
                    keyEndIndex++;

                buffKeyName = qstr.left(keyEndIndex).toStdString();

                if (qstr.left(keyEndIndex).toUpper() == "END")
                {
                    if (buffSectionName == "")
                        throw ExceptionExt("Unexpected \"END\" of section in [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());

                    // Write data to class and clear buffer variables.
                    if (sectionIndex == Sections.size())
                        Sections.push_back({buffSectionName, buffKeys});
                    else
                        Sections[sectionIndex].Keys = buffKeys;
                    buffSectionName = buffKeyName = buffKeyValue = "";
                    buffKeys.erase(buffKeys.cbegin(), buffKeys.cend());
                    continue;
                }

                if (keyEndIndex == 0)
                    throw ExceptionExt("Unexpected \"=\" sign in [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());

                if (buffSectionName == "")
                {
                    if (qstr.indexOf('=') != -1)
                        throw ExceptionExt("Unexpected key outside of section in [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());

                    buffSectionName = qstr.simplified().toStdString();

                    // Repeated blocks update the same record in CommandMap.ini.
                    for (sectionIndex = 0; sectionIndex < Sections.size(); sectionIndex++)
                        if (Sections[sectionIndex].Name == buffSectionName)
                        {
                            buffKeys = Sections[sectionIndex].Keys;
                            break;
                        }

                    continue;
                }

                // A second block of the same type must not replace an unclosed block.
                if (buffKeyName == QString::fromStdString(buffSectionName).section(' ', 0, 0).toStdString())
                    throw ExceptionExt("Missing \"END\" of section [" + buffSectionName + "] before [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());

                int keyValueIndex = keyEndIndex;
                while (keyValueIndex < qstr.size() && (qstr[keyValueIndex] == ' ' ||
                       qstr[keyValueIndex] == '\t' || qstr[keyValueIndex] == '='))
                    keyValueIndex++;

                if (keyValueIndex == qstr.size())
                    throw ExceptionExt("Missing value in [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());

                buffKeyValue = qstr.mid(keyValueIndex).trimmed().toStdString();

                bool keyFound = false;
                for (auto& key : buffKeys)
                    if (key.Name == buffKeyName)
                    {
                        key.Value = buffKeyValue;
                        keyFound = true;
                        break;
                    }

                if (!keyFound)
                    buffKeys.push_back({buffKeyName, buffKeyValue});
            }

            if (file.bad() || !file.eof())
                throw ExceptionExt("Unable to read file \"" + Path + "\" at line " + QString::number(fileLineIndex).toStdString());

            if (buffSectionName != "")
                throw ExceptionExt("Missing \"END\" of section [" + buffSectionName + "] at line " + QString::number(fileLineIndex).toStdString());

            LOGMSG( "File \"" + Path + "\" has been parsed; Sections count: " + Sections.size());
        }
        else
        {
            throw ExceptionExt("Bad file name; unable to open file \"" + Path + "\"");
        }

        file.close();
    }

    void GINIParser::Save()
    {
        Save(Path);
    }

    void GINIParser::Save(const string& strFileSample)
    {
        ofstream file(strFileSample, ios::out);

        LOGMSG("Attempt to write file \"" + strFileSample + "\"...");

        if (file.is_open())
        {
            for (const auto& elem : Sections)
            {
                file << elem.Name << endl;

                for (const auto& key : elem.Keys)
                    file << "  " << key.Name << " = " << key.Value << endl;

                file << "End" << endl;
            }

            file.close();
            if (file.fail())
                throw ExceptionExt("Unable to finish writing file \"" + strFileSample + "\"");

            LOGMSG("File \"" + strFileSample + "\" has been saved.");
        }
        else
        {
            throw ExceptionExt("Unable write settings to file \"" + strFileSample + "\"; make sure program have right to write into folder and into file.");
        }
    }

    void GINIParser::Save(const char* strFileSample)
    {
        Save(string(strFileSample));
    }

    void GINIParser::Save(const QString& strFileSample)
    {
        Save(strFileSample.toStdString());
    }
#pragma endregion

#pragma region Getters and setter
    vector<string> GINIParser::GetSectionsNames() const
    {
        vector<string> tmp;

        for (const auto& elem : Sections)
            tmp.push_back(elem.Name);

        return tmp;
    }

    vector<string> GINIParser::GetSectionKeys(const string& strSectionName) const
    {
        vector<string> tmp;

        for (const auto& elem : Sections)
            if (elem.Name == strSectionName)
            {
                for (const auto& key : elem.Keys)
                    tmp.push_back(key.Name);

                break;
            }
        
        return tmp;
    }

    vector<string> GINIParser::GetSectionKeys(const char* strSectionName) const
    {
        return GetSectionKeys(string(strSectionName));
    }

    vector<string> GINIParser::GetSectionKeys(const QString& strSectionName) const
    {
        return GetSectionKeys(strSectionName.toStdString());
    }

    string GINIParser::GetSectionValue(const string& strSectionName, const string& strSectionKey) const
    {
        string tmp;

        for (const auto& elem : Sections)
            if (elem.Name == strSectionName)
            {
                for (const auto& key : elem.Keys)
                    if (key.Name == strSectionKey)
                    {
                        tmp = key.Value;
                        break;
                    }

                break;
            }

        return tmp;
    }

    string GINIParser::GetSectionValue(const char* strSectionName, const char* strSectionKey) const
    {
        return GetSectionValue(string(strSectionName), string(strSectionKey));
    }

    QString GINIParser::GetSectionValue(const QString& strSectionName, const QString& strSectionKey) const
    {
        return QString::fromStdString(GetSectionValue(strSectionName.toStdString(), strSectionKey.toStdString()));
    }

    void GINIParser::SetSectionValue(const string& strName, const string& strKey, const string& strValue)
    {
        for (auto& elem : Sections)
            if (elem.Name == strName)
            {
                for (auto& key : elem.Keys)
                    if (key.Name == strKey)
                    {
                        key.Value = strValue;
                        break;
                    }

                break;
            }
    }

    void GINIParser::SetSectionValue(const char* strName, const char* strKey, const char* strValue)
    {
        SetSectionValue(string(strName), string(strKey), string(strValue));
    }

    void GINIParser::SetSectionValue(const QString& strName, const QString& strKey, const QString& strValue)
    {
        SetSectionValue(strName.toStdString(), strKey.toStdString(), strValue.toStdString());
    }
#pragma endregion
