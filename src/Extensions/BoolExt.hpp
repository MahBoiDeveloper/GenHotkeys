#pragma once
#include <concepts>
#include <QCheckBox>

namespace BoolExt
{
    template<class T>
    concept IsStateDescriptor = std::same_as<T, bool> || std:: same_as<T, Qt::CheckState>;

    template<IsStateDescriptor T>
    constexpr bool ToBool(const T state)
    {
        if constexpr (std::same_as<T, bool>)
        {
            return state;
        }
        else
        {
            switch (state)
            {
                case (Qt::CheckState::Checked):
                    return true;
                case (Qt::CheckState::Unchecked):
                    return false;
                default:
                    return false;
            }
        }
    }
}
