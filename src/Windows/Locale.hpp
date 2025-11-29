#pragma once
#include <QString>
#include "../ProgramConstants.hpp"

namespace Windows
{
    class Locale final
    {
    public:
        /// @brief Returns full locale name by the given partial name (e.g. "en" -> "en-US")
        static QString GetFullLocaleName(const QString& partial);
        /// @brief Returns English language name (e.g. "en-US" -> "English")
        static QString LanguageName(const QString& part);
        /// @brief Return short language name text equivalent of the enum Languages value.
        static QString GetLanguageShortName(Languages language);
        /// @brief Return full language name text equivalent of the enum Languages value.
        static QString GetLanguageFullName(Languages language);
        /// @brief Return enum Languages value that equivalent to the text value.
        static Languages LangEnumFromLocale(const QString& locale);
    };
}
