#include "Windows/Locale.hpp"
#include "Convert.hpp"
using namespace std;

namespace Convert
{
    QStringList ToQStringList(const list<string>& srcList)
    {
        QStringList retList;

        for (const auto& elem : srcList)
            retList.push_back(QString::fromStdString(elem));

        return retList;
    }

    list<string> ToStdStringList(const QStringList& srcList)
    {
        list<string> retList;
    
        for (const auto& elem : srcList)
            retList.push_back(elem.toStdString());
    
        return retList;
    }

    Languages LangEnumFromLocale(const QString& locale)
    {
        QString lowerLocale = locale.toLower();

        for(auto elem = PROGRAM_CONSTANTS->LANGUAGES_STRINGS.cbegin(); elem != PROGRAM_CONSTANTS->LANGUAGES_STRINGS.cend(); ++elem)
            if (Windows::Locale::GetLanguageShortName(elem.key()) == lowerLocale)
                return elem.key();
    
        return Languages::English;
    }

    Languages ToLangEnum(const std::string& locale) { return LangEnumFromLocale(QString::fromStdString(locale)); }
    Languages ToLangEnum(const QString& locale)     { return LangEnumFromLocale(locale); }
    Qt::Key   ToQtKey(const QChar& ch)              { return static_cast<Qt::Key>(QChar(ch).unicode()); }
    
    QString ToQString(const Languages& locale)
    {
        for(auto elem = PROGRAM_CONSTANTS->LANGUAGES_STRINGS.cbegin(); elem != PROGRAM_CONSTANTS->LANGUAGES_STRINGS.cend(); ++elem)
            if (elem.key() == locale)
                return elem.value().first;
        
        return "en";
    }
}
