#pragma once
#include <string>
#include <QString>
#include <QJsonObject>

class JSONFile final
{
private: // Data
    QJsonObject JsonMainObject;

public: // Methods
    explicit JSONFile();
    explicit JSONFile(const QString&     filePath);
    explicit JSONFile(const std::string& filePath);
    explicit JSONFile(const char*        filePath);

    /// @brief Returns main object of parsed JSON file
    const QJsonObject&  GetMainObject() const;
    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    QJsonValue          Query(const QString&     strQuery) const;
    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    QJsonValue          Query(const std::string& strQuery) const;
    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    QJsonValue          Query(const char*        strQuery) const;
    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    static QJsonValue   Query(const QJsonObject&, const char*    strQuery);
    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    static QJsonValue   Query(const QJsonObject&, const QString& strQuery);

    /// @brief Write via LOGMSG information about value. Use it only if debug.
    static void LogInfoAboutValue(const QJsonValue& val);
};
