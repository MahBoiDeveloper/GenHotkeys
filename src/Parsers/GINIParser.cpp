#include <QStringList>

#include "../Logger.hpp"
#include "../Exception.hpp"
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

                int equalSignIndex = qstr.indexOf('=');
                
                switch (equalSignIndex)
                {
                    case -1: // Read line may be a begin or end of section

                        // If line equals END and section name doesn't set, then throw exception
                        if ((qstr.toUpper() == "END") && (buffSectionName == ""))
                            throw Exception(string("Unexpected \"END\" of section in [") + buff + 
                                            string("] at line ") + 
                                            QString::number(fileLineIndex).toStdString());

                        // If line doesn't equal END, then it is a section name
                        if (qstr.toUpper() != "END")
                        {
                            buffSectionName = qstr.toStdString();
                            continue;
                        }

                        // If line equals END and section name has been set, then write data into class and clear buffers
                        if ((qstr.toUpper() == "END") && (buffSectionName != ""))
                        {
                            // Write data to class
                            Sections.push_back({buffSectionName, buffKeys});

                            // Clear buffer variables
                            buffSectionName = buffKeyName = buffKeyValue = "";
                            buffKeys.erase(buffKeys.cbegin(), buffKeys.cend());

                            continue;
                        }

                        break;

                    case 0: // Error due to only equal sign in line
                        throw Exception("Unexpected \"=\" sign in [" + buff + "] at line " + QString::number(fileLineIndex).toStdString());
                        break;
                
                    default: // Read line is a value
                        auto tmp = qstr.split('=');
                        buffKeyName  = tmp[0].trimmed().toStdString();
                        buffKeyValue = tmp[1].trimmed().toStdString();
                        buffKeys.push_back({buffKeyName, buffKeyValue});
                        break;
                }
            }

            LOGMSG( "File \"" + Path + "\" has been parsed; Sections count: " + GINIParser::Instance->Sections.size());
        }
        else
        {
            throw Exception("Bad file name; unable to open file \"" + Path + "\"");
        }

        file.close();
    }

    void GINIParser::Save()
    {
        ofstream file(Path, ios::out);

        LOGMSG("Attempt to write file \"" + Path + "\"...");

        if (file.is_open())
        {
            for (const auto& elem : Sections)
            {
                file << elem.Name << endl;

                for (const auto& key : elem.Keys)
                    file << "  " << key.Name << " = " << key.Value << endl;

                file << "End" << endl;
            }

            LOGMSG("File \"" + Path + "\" has been saved.");
        }
        else
        {
            throw Exception("Unable write settings to file \"" + Path + "\"; make sure program have right to write into folder and into file.");
        }
    }

    void GINIParser::Save(const string& strFileSample)
    {
        string tmp = Path;
        Path = strFileSample;
        Save();
        Path = tmp;
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
