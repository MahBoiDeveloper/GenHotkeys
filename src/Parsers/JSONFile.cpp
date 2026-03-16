#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include "JSONFile.hpp"
#include "../Exception.hpp"
#include "../Logger.hpp"

using namespace std;
using namespace StringExt;

#pragma region CTORs and DTORs
    JSONFile::JSONFile()
    {
    }
    
    JSONFile::JSONFile(const QString& filePath)
    {
        QFile openedFile(filePath);
        QJsonParseError err;

        // Read data from *.json file
        if (openedFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            LOGMSG("Parsing \"" + filePath + "\"...");
            JsonMainObject = QJsonDocument::fromJson(openedFile.readAll(), &err).object();
            openedFile.close();
            LOGMSG("Errors while parsing: " + err.errorString());
            LOGMSG("JSON file has been parsed");
        }
        else
        {
            LOGMSG("Errors while parsing: " + err.errorString());
            throw Exception("Bad file name; unable to open file \"" + QString{filePath} + "\"");
        }
    }

    JSONFile::JSONFile(const char* filePath)        {*this = JSONFile(QString(filePath));}
    JSONFile::JSONFile(const std::string& filePath) {*this = JSONFile(QString::fromStdString(filePath));}
#pragma endregion

#pragma region Getters
    const QJsonObject& JSONFile::GetMainObject() const { return JsonMainObject; }
    
    QJsonValue JSONFile::Query(const QString&     strQuery) const { return Query(JsonMainObject, strQuery); }
    QJsonValue JSONFile::Query(const char*        strQuery) const { return Query(JsonMainObject, QString{strQuery}); }
    QJsonValue JSONFile::Query(const std::string& strQuery) const { return Query(JsonMainObject, QString::fromStdString(strQuery)); }

    QJsonValue JSONFile::Query(const QJsonObject& jsonObject, const QString& strQuery)
    {
        // Find dollar sign in place of the first character
        if (strQuery.at(0) != '$')
        {
            throw Exception("JSON path doesn't begin with \'$\'"q);
        }

        QStringList splitList = strQuery.split('.');
        splitList.removeFirst();
        LOGSTM << "Splited and updated query has length : " << splitList.length() << endl;

        QJsonObject currObj = jsonObject;
        QJsonValue  currVal;

        for (int iter = 0; iter < splitList.length(); iter++)
        {
            QString currSplit = splitList.at(iter);
            LOGSTM << "Current search value : [" << currSplit.toStdWString() << ']' << endl;

            // Current value actually is array
            if (currSplit.contains('[') && currSplit.contains(']'))
            {
                const QRegularExpression regexp{"\\[\\d+\\]"};

                // Find [xxxx] number of index in array and clear string from [] bracket
                int arrayIndex = regexp.match(currSplit).captured(0).remove('[').remove(']').toInt();
                
                // Try to find QJsonValue in array
                currVal = currObj.value(currSplit.remove(currSplit.indexOf('['),
                                                         currSplit.length() - currSplit.indexOf('[')))
                                                         .toArray()
                                                         .at(arrayIndex);
            }
            // Current value may be an object
            else
            {
                currVal = currObj.value(currSplit);
            }

            // If not last - then object/array
            if (iter != splitList.length() - 1)
            {
                currObj = currVal.toObject();
            }
        }

        return currVal;
    }

    QJsonValue JSONFile::Query(const QJsonObject& jsonObject, const char* strQuery)
    {
        return Query(jsonObject, QString(strQuery));
    }

    void JSONFile::LogInfoAboutValue(const QJsonValue& val)
    {
        LOGMSG("Information about QJsonValue value:");
        LOGMSG("\tValue is Array?     - " + (val.isArray()     ? "True" : "False"));
        LOGMSG("\tValue is Bool?      - " + (val.isBool()      ? "True" : "False"));
        LOGMSG("\tValue is Double?    - " + (val.isDouble()    ? "True" : "False"));
        LOGMSG("\tValue is Null?      - " + (val.isNull()      ? "True" : "False"));
        LOGMSG("\tValue is Object?    - " + (val.isObject()    ? "True" : "False"));
        LOGMSG("\tValue is String?    - " + (val.isString()    ? "True" : "False"));
        LOGMSG("\tValue is Undefined? - " + (val.isUndefined() ? "True" : "False"));
        LOGMSG("\tLength of array is : "  + val.toArray().size());
    }
#pragma endregion
