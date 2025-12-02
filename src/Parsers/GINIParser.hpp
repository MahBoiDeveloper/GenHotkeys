#pragma once
#include <vector>
#include <memory>
#include <QString>

class GINIParser final
{
public: // Data
    struct GINIKey final
    {
        std::string Name;
        std::string Value;
    };

    struct GINISection final
    {
        std::string          Name;
        std::vector<GINIKey> Keys;
    };

    inline static std::unique_ptr<GINIParser> Instance;
private:
    std::string              Path;
    std::vector<GINISection> Sections;

public: // Methods
    explicit GINIParser() = delete;
    explicit GINIParser(const char* strFileSample);
    explicit GINIParser(const std::string& strFileSample);
    explicit GINIParser(const QString& strFileSample);

    /// @brief Save .ini file.
    void Save();
    /// @brief Save .ini file.
    void Save(const std::string& strFileSample);
    /// @brief Save .ini file.
    void Save(const char* strFileSample);
    /// @brief Save .ini file.
    void Save(const QString& strFileSample);

    /// @brief Returns all section names from .ini file.
    std::vector<std::string> GetSectionsNames()                                                                    const;
    /// @brief Returns all keys for direct section from .ini file.
    std::vector<std::string> GetSectionKeys  (const char* strSectionName)                                          const;
    /// @brief Returns all keys for direct section from .ini file.
    std::vector<std::string> GetSectionKeys  (const std::string& strSectionName)                                   const;
    /// @brief Returns all keys for direct section from .ini file.
    std::vector<std::string> GetSectionKeys  (const QString& strSectionName)                                       const;
    /// @brief Returns text value from section's key.
    std::string              GetSectionValue (const char* strSectionName, const char* strSectionKey)               const;
    /// @brief Returns text value from section's key.
    std::string              GetSectionValue (const std::string& strSectionName, const std::string& strSectionKey) const;
    /// @brief Returns text value from section's key.
    QString                  GetSectionValue (const QString& strSectionName, const QString& strSectionKey)         const;

    /// @brief Set new value for section's key.
    void SetSectionValue(const std::string& strName, const std::string& strKey, const std::string& strValue);
    /// @brief Set new value for section's key.
    void SetSectionValue(const char* strName, const char* strKey, const char* strValue);
    /// @brief Set new value for section's key.
    void SetSectionValue(const QString& strName, const QString& strKey, const QString& strValue);
private:
    /// @brief Parsing Generals Zero Hour .ini file format.
    void Parse();
};
