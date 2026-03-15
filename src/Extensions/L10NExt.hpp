#include <string>
#include <QString>
#include <QTranslator>

/// @brief Shortcut for StringExt::l10n(x, "QObject")
#define L10N(x) L10NExt::l10n(x)

namespace L10NExt
{
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const char*         string, const QString& object = "QObject");
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const QString&      string, const QString& object = "QObject");
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const std::string&  string, const QString& object = "QObject");
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const std::wstring& string, const QString& object = "QObject");
}
