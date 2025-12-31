#include <QString>
#include <QVector>
#include <QByteArray>
#include <QStringList>
#include <list>
#include <windows.h>

#include "../StringExt.hpp"
#include "Locale.hpp"

using namespace std;

namespace Windows
{
    QString Locale::GetFullLocaleName(const QString& partial)
    {
        std::wstring wpartial = partial.toStdWString();
        WCHAR name[LOCALE_NAME_MAX_LENGTH] = {};

        // Try as-is first
        if (GetLocaleInfoEx(wpartial.c_str(), LOCALE_SNAME, name, LOCALE_NAME_MAX_LENGTH))
            return partial;

        // Collect all locales
        std::vector<std::wstring> locales;
        EnumSystemLocalesEx([](LPWSTR lpLocaleString, DWORD, LPARAM lParam) -> BOOL
        {
            auto* locales = reinterpret_cast<std::vector<std::wstring>*>(lParam);
            locales->emplace_back(lpLocaleString);
            return TRUE;
        }, LOCALE_ALL, reinterpret_cast<LPARAM>(&locales), nullptr);

        // Find one that starts with the partial name
        for (const auto& loc : locales)
        {
            if (_wcsnicmp(loc.c_str(), wpartial.c_str(), wpartial.size()) == 0)
                return QString::fromStdWString(loc);
        }

        return StringExt::EmptyString;
    }

    QString Locale::LanguageName(const QString& partial)
    {
        QString locale = GetFullLocaleName(partial);
        
        if (locale.isEmpty())
            return StringExt::EmptyString;

        std::wstring wlocale = locale.toStdWString();
        WCHAR buffer[100] = {};

        if (GetLocaleInfoEx(wlocale.c_str(), LOCALE_SENGLISHLANGUAGENAME,
                            buffer, static_cast<int>(std::size(buffer))))
        {
            return QString::fromWCharArray(buffer);
        }

        return StringExt::EmptyString;
    }
}
