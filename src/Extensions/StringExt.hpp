#pragma once
#include <concepts>
#include <QString>

/// @brief Shortcut for QString(#x).
/// @attention Macro converting any text to the string, not the specific types/variables/functions.
#define nameof(x) QString(#x)

/// @brief QString, std-strings and c-strings extension.
namespace StringExt
{
    inline const QString EmptyString("");

    template<class T>
    concept IsIntegerOrFloat =    std::same_as<T, long>      || std::same_as<T, int>
                               || std::same_as<T, qlonglong> || std::same_as<T, qulonglong>
                               || std::same_as<T, uint>      || std::same_as<T, ulong>
                               || std::same_as<T, double>    || std::same_as<T, float>;

    template<class T>
    concept IsInteger =    std::same_as<T, long>        || std::same_as<T, int>
                        || std::same_as<T, qlonglong>   || std::same_as<T, qulonglong>
                        || std::same_as<T, uint>        || std::same_as<T, ulong>
                        || std::same_as<T, std::size_t> || std::same_as<T, ushort>;

    template<class T>
    concept IsSymbol = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, QChar>;

    template<class T>
    concept IsCString = std::same_as<T, const char*> || std::same_as<T, const wchar_t*>;

    template<class T>
    concept IsSTDString = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

    template<class T>
    concept IsString = std::same_as<T, const char*> || std::same_as<T, const wchar_t*> || 
                       std::same_as<T, std::string> || std::same_as<T, std::wstring>   || 
                       std::same_as<T, QString>;
    
    /// @brief Converts std-strings and c-strings to the `QString`.
    template<IsString S>
    inline constexpr QString ToQString(const S& str)
    {
        if      constexpr (std::same_as<S, QString>)      return str;
        else if constexpr (std::same_as<S, std::string>)  return QString::fromStdString(str);
        else if constexpr (std::same_as<S, std::wstring>) return QString::fromStdWString(str);
        else if constexpr (std::same_as<S, char*>)        return QString::fromStdString(std::string(str));
        else if constexpr (std::same_as<S, wchar_t*>)     return QString::fromStdWString(std::wstring(str));
        else                                              return EmptyString;
    }

    /// @brief Converts characters to the `QString`.
    template<IsSymbol C>
    inline constexpr QString ToQString(const C& ch)
    {
        if      constexpr (std::same_as<C, QChar>)   return QString(ch);
        else if constexpr (std::same_as<C, char>)    return QString(QChar(ch));
        else if constexpr (std::same_as<C, wchar_t>) return QString(QChar(ch));
        else                                         return EmptyString;
    }

    /// @brief Converts `size_t`-like integer types to the `QString`.
    inline QString ToQString(const size_t num) { return QString::number(num); }
    
    /// @brief Converts `QChar` to the `wchar_t`.
    template<IsSymbol C>
    inline wchar_t ToWchar(const C& ch)
    {
        if      constexpr (std::same_as<C, wchar_t>) return ch;
        else if constexpr (std::same_as<C, char>)    return static_cast<wchar_t>(ch);
        else if constexpr (std::same_as<C, QChar>)   return static_cast<wchar_t>(ch.unicode());
        else                                         return L'\0';
    }

    /// @brief Checks if character is ASCII-compitable.
    template<IsSymbol T>
    inline bool IsACII(const T& chr) { return QChar(chr).unicode() <= 127; }

    /// @brief Checks if all characters in string are ASCII-compitable.
    template<IsString T>
    inline bool IsACII(const T& str)
    {
        for(const QChar& ch : ToQString(str))
        {
            if (ch.unicode() > 127)
            {
                return false;
            }
        }

        return true;
    }

    // Doesn't work due to attempt of redefinition pointers mathematic.
    // template<IsCString S, IsCString T> inline QString operator+ (const S& str1, const T& str2) { return EmptyString; }

    template<IsString S,  IsString T>  inline constexpr QString operator+ (const S& str1, const T& str2)   { return ToQString(str1).append(ToQString(str2)); }
    template<IsString S,  IsInteger N> inline constexpr QString operator+ (const S& str,  const N& num)    { return ToQString(str).append(QString::number(num)); }
    template<IsInteger N, IsString S>  inline constexpr QString operator+ (const N& num,  const S& str)    { return QString::number(num).append(ToQString(str)); }
    template<IsString S,  IsSymbol C>  inline constexpr QString operator+ (const S& str,  const C& ch)     { return ToQString(str).append(ToQString(ch)); }
    template<IsSymbol C,  IsString S>  inline constexpr QString operator+ (const S& str,  const C& ch)     { return ToQString(str).append(ToQString(ch)); }
    template<IsString S>               inline constexpr QString operator+ (const S& str, const bool& flag) { return ToQString(str).append(flag ? "true" : "false"); }

    // Ingores ""q instead of ""_q. Thank you C++ committee for another useless warning.
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wliteral-suffix"
    /// @brief Works fine, but not a compile-time optimized.
    /// @brief To be able optimize it in compile-time update to Qt 6.4 >= due to constexpr ctor in QString class.
    /// @brief Reference: https://doc.qt.io/qt-6/qstring.html#operator-22-22_s
    /// @example QString qstr = "text"q + "sample";
    /// @attention It is possible to use only if `using namespace StringExt;` was declared!
    inline QString operator""q (const char*    str, size_t len) noexcept { return QString::fromUtf8(str, len); }
    
    /// @brief Look at `""q` example with `const char*`.
    inline QString operator""q (const wchar_t* str, size_t len) noexcept { return QString::fromWCharArray(str, len); }
    #pragma GCC diagnostic pop
}
