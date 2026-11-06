#include <QString>
#include <QRegExp>
#include "../Extensions/StringExt.hpp"
#include "../Core/Logger.hpp"

namespace TextParser
{
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    template<StringExt::IsString T>
    inline wchar_t GetHotkeyWchar(const T& input)
    {
        QString strName = StringExt::ToQString(input);
        wchar_t hk      = L' ';
        size_t  index   = 0;

        index = strName.indexOf(L'&');
        if (index != -1 )
        {
            hk = strName.toUpper().toStdWString().at(index + 1);
        }

        return hk;
    }

    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    template<StringExt::IsString T>
    inline QChar GetHotkey(const T& strName) { return QChar(GetHotkeyWchar(StringExt::ToQString(strName))); }
    
    /// @brief
    template<StringExt::IsString T, StringExt::IsSymbol C>
    inline QString SetHotkey(const T& str, const C& ch)
    {
        QString text = StringExt::ToQString(str);
        wchar_t wchLetter = StringExt::ToWchar(ch);

        if (wchLetter == L' ')
        {
            return text;
        }

        if (!text.contains(L'&') || text.size() <= 4) // if not "[&F]"
        {
            text = QString("[&") + QString(wchLetter) + "] " + text;
        }
        else
        {
            text[text.indexOf(L'&') + 1] = wchLetter;
        }

        return text;
    }

    /// @brief Remove hotkey markers and standalone hotkey islands from an action name.
    inline QString GetClearActionName(QString strName) { return strName.remove(QRegExp{"\\[&.\\]|\\(&.\\)|&"}).trimmed(); }

    /// @brief Move an existing hotkey to a single [&H] prefix, preserving the action name.
    inline QString ExtractHotkey(const QString& text)
    {
        const int markerIndex = text.indexOf('&');
        if (markerIndex < 0 || markerIndex + 1 >= text.size())
            return text;

        const QChar hotkey = text.at(markerIndex + 1).toUpper();
        if (hotkey.isSpace() || hotkey.isNull() || hotkey == '&')
            return text;

        return SetHotkey(GetClearActionName(text), hotkey);
    }
}
