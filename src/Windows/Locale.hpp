#pragma once
#include <QString>

namespace Windows
{
    class Locale final
    {
    public:
        static QString GetFullLocaleName(const QString& partial);
        static QString LanguageName(const QString& part);
    };
}
