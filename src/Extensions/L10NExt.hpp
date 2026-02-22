#include <string>
#include <QString>
#include <QTranslator>

/// @brief Shortcut for StringExt::l10n()
#define L10N(x) L10NExt::l10n(x)

namespace L10NExt
{
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const char*         string);
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const QString&      string);
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const std::string&  string);
    /// @brief Shortcut for QCoreApplication::translate("QObject", str)
    QString l10n(const std::wstring& string);
}
