#pragma once
#include <QString>

namespace Windows
{
    class Locale final
    {
    public:
        /// @brief Returns full locale name by the given partial name (e.g. "en" -> "en-US")
        static QString GetFullLocaleName(const QString& partial);
        /// @brief Returns English language name (e.g. "en-US" -> "English")
        static QString LanguageName(const QString& part);
    };
}
