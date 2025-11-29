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

    Qt::Key   ToQtKey(const QChar& ch) { return static_cast<Qt::Key>(QChar(ch).unicode()); }
}
