#pragma once
#include <string>
#include <list>
#include <QStringList>
#include "ProgramConstants.hpp"

namespace Convert
{
    /// @brief Returns list `QStringList` of `QString` as element by converting `std::list<std::string>` value.
    QStringList ToQStringList(const std::list<std::string>& srcList);
    /// @brief Returns list `std::list` of `std::string` as element by converting `QStringList<QString>` value.
    inline std::list<std::string> ToStdStringList(const QStringList& srcList);
    /// @brief Converts `QChar` to `Qt::Key` enum equivalent.
    Qt::Key ToQtKey(const QChar& ch);
}
