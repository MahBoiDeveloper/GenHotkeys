#include <QCoreApplication>
#include "StringExt.hpp"
#include "L10NExt.hpp"

QString L10NExt::l10n(const char*         string, const QString& object) { return QCoreApplication::translate(object.toStdString().c_str(), string); }
QString L10NExt::l10n(const QString&      string, const QString& object) { return QCoreApplication::translate(object.toStdString().c_str(), StringExt::ToQString(string).toStdString().c_str()); }
QString L10NExt::l10n(const std::string&  string, const QString& object) { return QCoreApplication::translate(object.toStdString().c_str(), StringExt::ToQString(string).toStdString().c_str()); }
QString L10NExt::l10n(const std::wstring& string, const QString& object) { return QCoreApplication::translate(object.toStdString().c_str(), StringExt::ToQString(string).toStdString().c_str()); }
