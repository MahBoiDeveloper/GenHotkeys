#pragma once
#include <list>
#include <memory>
#include <string>
#include <fstream>

#define CSF_PARSER CSFParser::Instance

class CSFParser final
{
public: // Types
    struct CompiledString
    {
        QString Name;
        QString Value;
    };

    struct HotkeyAssociation
    {
        QString StringName;
        wchar_t HotkeyLetter;
    };

    struct CSFHeader
    {
        uint8_t  csfChars[4];
        uint32_t formatVersion;
        uint32_t numberOfLabels;
        uint32_t numberOfStrings;
        uint32_t uselessBytes;
        uint32_t languageCode;
    };

private: //Data
    const uint8_t  FSC[4]  {' ', 'F', 'S', 'C'}; // Begining of any CSF file header
    const uint8_t  LBL[4]  {' ', 'L', 'B', 'L'}; // Begining of any string name aka "label"
    const uint8_t  RTS[4]  {' ', 'R', 'T', 'S'}; // Begining of any string value aka "string"
    const uint8_t  WRTS[4] {'W', 'R', 'T', 'S'}; // Begining of any string with extra value
    const uint32_t CNC_CSF_VERSION = 3;          // Standart file format. Legacy by WW
    
    std::wstring              Path;
    CSFHeader                 Header;
    std::list<CompiledString> Table;

public:
    inline static std::unique_ptr<CSFParser> Instance = nullptr;

private: // Methods
    void ReadHeader(std::ifstream* csfFile);
    void ReadBody(std::ifstream* csfFile);

    void WriteHeader(std::ofstream* csfFile);
    void WriteBody(std::ofstream* csfFile);

    /// @brief Pure function-convertor from char array to std::string.
    std::string  CharArrayToString(const size_t& arrayLength, const char* pArray)     const;
    /// @brief Pure function-convertor from wchar_t array to std::wstring.
    std::wstring WharArrayToWstring(const size_t& arrayLength, const wchar_t* pArray) const;

public:
    explicit CSFParser();
    explicit CSFParser(const std::wstring& strFilePath);
    explicit CSFParser(const std::string&  strFilePath);
    explicit CSFParser(const char*         strFilePath);
    explicit CSFParser(const QString&      strFilePath);

    /// @brief Parse .csf file with direct path.
    void Parse(const std::wstring& strFilePath);
    /// @brief Parse .csf file with direct path.
    void Parse(const std::string&  strFilePath);
    /// @brief Parse .csf file with direct path.
    void Parse(const char*         strFilePath);
    /// @brief Parse .csf file with direct path.
    void Parse(const QString&      strFilePath);

    /// @brief Save compiled sting table data to the parsed file before.
    void Save();
    /// @brief Save compiled string table data to the specific file.
    void Save(const std::wstring& strFileName);
    /// @brief Save compiled string table data to the specific file.
    void Save(const std::string&  strFileName);
    /// @brief Save compiled string table data to the specific file.
    void Save(const char*         strFileName);
    /// @brief Save compiled string table data to the specific file.
    void Save(const QString&      strFileName);

    /// @brief Returns first string value by name match. The same string in uppercase and in lowercase aren't identical.
    std::wstring              GetStringValue(const std::string& strName)                                       const;
    /// @brief Returns first string value by name match. The same string in uppercase and in lowercase aren't identical.
    QString                   GetStringValue(const QString& strName)                                           const;
    /// @brief Returns first string value by name match. The same string in uppercase and in lowercase aren't identical.
    QString                   GetStringValue(const char* strName)                                              const;
    /// @brief Returns list of all strings in category with short name (substrings after ':').
    QStringList               GetCategoryStrings(const QString& strCategoryName)                               const;
    /// @brief Returns list of all strings in category.
    QStringList               GetCategoryStringsWithFullNames(const QString& strCategoryName)                  const;
    /// @brief Returns all string names in compiled sting table.
    std::list<std::string>    GetStringNames()                                                                 const;
    /// @brief Returns list of categories (substrings before ':').
    std::list<std::string>    GetCategories()                                                                  const;
    /// @brief Returns list of all strings in category with short name (substrings after ':').
    std::list<std::string>    GetCategoryStrings(const std::string& strCategoryName)                           const;
    /// @brief Returns list of all strings in category.
    std::list<std::string>    GetCategoryStringsWithFullNames(const std::string& strCategoryName)              const;
    /// @brief Returns list of all strings that values contains wide character.
    std::list<std::string>    GetStringsContainsSymbol(const wchar_t& wch)                                     const;
    /// @brief Returns list of all strings that values contains wide character in specific category.
    std::list<std::string>    GetStringsContainsSymbol(const wchar_t& wch, const std::string& strCategoryName) const;
    /// @brief Returns list of full strings data.
    std::list<CompiledString> GetStringsByNameList(const std::list<std::string>& lstNames)                     const;

    /// @brief Delete all text by regular expression \[&[A-Z]\].
    QString GetClearName(const QString& strName)                                                               const;

    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    QChar   GetHotkey(const std::string& strName)                                                              const;
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    QChar   GetHotkey(const char* strName)                                                                     const;
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    QChar   GetHotkey(const QString& strName)                                                                  const;
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    wchar_t GetHotkeyWchar(const std::string& strName)                                                         const;
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    wchar_t GetHotkeyWchar(const char* strName)                                                                const;
    /// @brief Returns wide character (letter after & sign) assinged to keyboard key.
    wchar_t GetHotkeyWchar(const QString& strName)                                                             const;
    /// @brief Returns list of data structs with string names and its keyboard key assignment.
    std::list<HotkeyAssociation> GetHotkeys(const std::list<std::string>& lstStringNames)                      const;

    /// @brief Searchs any match for string name and rewriting hotkey assignment for it.
    void SetHotkey(const std::string& strName, const wchar_t& wchLetter);
    /// @brief Searchs any match for string name and rewriting hotkey assignment for it.
    void SetHotkey(const char* strName, const wchar_t& wchLetter);
    /// @brief Searchs any match for string name and rewriting hotkey assignment for it.
    void SetHotkey(const QString& strName, const wchar_t& wchLetter);
    /// @brief Searchs any match for string name and rewriting its value.
    void SetStringValue(const std::string& strName, const std::wstring& wstrValue);
    /// @brief Searchs any match for string name and rewriting its value.
    void SetStringValue(const CompiledString& stString);
    /// @brief Searchs any match for string names and rewriting its values.
    void SetStringsValue(const std::list<CompiledString>& lstChanges);

    /// @brief Checks if string exist by name match.
    const bool ExistString(const QString& value)       const;
    /// @brief Checks if string exist by name match.
    const bool ExistString(const char* value)          const;
    /// @brief Checks if string exist by name match.
    const bool ExistString(const std::string& value)   const;
    /// @brief Checks if category exist by name match.
    const bool ExistCategory(const QString& value)     const;
    /// @brief Checks if category exist by name match.
    const bool ExistCategory(const char* value)        const;
    /// @brief Checks if category exist by name match.
    const bool ExistCategory(const std::string& value) const;
};
