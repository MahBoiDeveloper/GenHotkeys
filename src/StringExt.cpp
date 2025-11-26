#include <QCoreApplication>
#include "StringExt.hpp"

QString StringExt::l10n(const char*         string) { return QCoreApplication::translate("QObject", QString(string).toUtf8().constData()); }
QString StringExt::l10n(const QString&      string) { return QCoreApplication::translate("QObject", string.toUtf8().constData()); }
QString StringExt::l10n(const std::string&  string) { return QCoreApplication::translate("QObject", QString::fromStdString(string).toUtf8().constData()); }
QString StringExt::l10n(const std::wstring& string) { return QCoreApplication::translate("QObject", QString::fromStdWString(string).toUtf8().constData()); }
