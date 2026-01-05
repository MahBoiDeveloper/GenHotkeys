#include <sstream>
#include <QStringList>

#include "../Logger.hpp"
#include "../Convert.hpp"
#include "../Exception.hpp"
#include "CSFParser.hpp"

using namespace std;
using namespace StringExt;

#pragma region CTORs and DTORs
    CSFParser::CSFParser()                        {}
    CSFParser::CSFParser(const wstring& filePath) { Parse(filePath); }
    CSFParser::CSFParser(const string&  filePath) { Parse(filePath); }
    CSFParser::CSFParser(const char*    filePath) { Parse(filePath); }
    CSFParser::CSFParser(const QString& filePath) { Parse(filePath); }
#pragma endregion

#pragma region Parsing
    void CSFParser::Parse(const char* strFilePath)
    {
        Path = QString(strFilePath).toUpper().toStdWString();
        ifstream file(Path.c_str(), ios::binary | ios::in);

        if (file.is_open())
        {
            if (Path.ends_with(L".BIG"))
            {
                LOGMSG("BIG archive detected. Try to find CSF file inside...");
                streampos offset = SetOffsetAtTheStringStart(file, " FSC"s);
                LOGMSG("CSF file data found at offset : " + static_cast<const uint64_t&>(offset));
            }

            LOGMSG("Attempt to read string table from \"" + Path.c_str() + "\" file...");
            ReadHeader(&file);
            ReadBody(&file);
            LOGMSG("CSF file has been parsed. Strings count: " + Table.size());
        }
        else
        {
            throw Exception("Bad file name error. Unable to open file \""q + Path + "\"");
        }
    }
    void CSFParser::Parse(const wstring& filePath) { Parse(QString::fromStdWString(filePath)); }
    void CSFParser::Parse(const string& filePath)  { Parse(QString::fromStdString(filePath)); }
    void CSFParser::Parse(const QString& filePath) { Parse(filePath.toStdString().c_str()); }

    streampos CSFParser::SetOffsetAtTheStringStart(ifstream& file, const string& searchStr)
    {
        const streampos startOffset = file.tellg();
        const streampos delta = sizeof(char) * searchStr.length();
        const streampos testDelta = 1;

        bool searchResult = false;
        string searchWindow = searchStr;
        char c = ' ';

        while (!searchResult && file.good())
        {
            file.read(&c, testDelta);
            if (searchStr[0] == c)
            {
                // Return back to 1 character
                file.seekg(file.tellg() - testDelta);
                file.read(const_cast<char*>(searchWindow.c_str()), delta);
                searchResult = searchStr == searchWindow;
                // Return back search window position
                file.seekg(file.tellg() - delta + testDelta);
            }
            else
                continue;
        }

        // Restore position before search string
        file.seekg(file.tellg() + delta - testDelta);

        if (!file.good())
        {
            file.clear();
            file.seekg(startOffset);
            LOGMSG(PROGRAM_CONSTANTS->CSF_NO_CSF_IN_BIG.arg(Path));
            throw Exception(L10N(PROGRAM_CONSTANTS->CSF_NO_CSF_IN_BIG).arg(Path));
        }

        streampos offset = file.tellg();
        file.clear();
        file.seekg(offset - delta);

        return offset;
    }

    void CSFParser::ReadHeader(ifstream* csfFile)
    {
        csfFile->read(reinterpret_cast<char*>(&Header), sizeof(Header));

        LOGSTM << "File header data:" << endl;

        LOGSTM << '\t' << "First 4th bytes of file header are  : [" << Header.csfChars[0] 
                                                                    << Header.csfChars[1] 
                                                                    << Header.csfChars[2] 
                                                                    << Header.csfChars[3] << ']' << endl;
        LOGSTM << '\t' << "CSF file format version             : "  << Header.formatVersion      << endl;
        LOGSTM << '\t' << "Number of labels in CSF file        : "  << Header.numberOfLabels     << endl;
        LOGSTM << '\t' << "Number of strings in CSF file       : "  << Header.numberOfStrings    << endl;
        LOGSTM << '\t' << "Useless bytes, i guess?             : "  << Header.uselessBytes       << endl;
        LOGSTM << '\t' << "Language code                       : "  << Header.languageCode       << endl;
    }

    void CSFParser::ReadBody(ifstream* csfFile)
    {
        uint8_t lbl[4];

        for (uint32_t i = 0; i < Header.numberOfLabels; i++)
        {
            bool breakFlag = false;

            // Reading ' LBL' characters
            csfFile->read(reinterpret_cast<char*>(&lbl), sizeof(lbl));

            // Comparing read characters with reference string
            for (uint8_t tmp = 0; tmp < 4; tmp++)
            {
                if (lbl[tmp] != LBL[tmp])
                {
                    if (i > 0) i--;
                    breakFlag = true;
                    break;
                }
            }
         
            // Works only if 4 chars are not equal ' LBL'
            if (breakFlag) continue;
         
            // Read count of strings linked to one string label (name)
            uint32_t countOfStrings;
            csfFile->read(reinterpret_cast<char*>(&countOfStrings), sizeof(countOfStrings));

            // Read string name without \0 sign
            uint32_t labelNameLength;
            csfFile->read(reinterpret_cast<char*>(&labelNameLength), sizeof(labelNameLength));

            // Read ASCI-like string name without \0 sign
            uint8_t labelName[labelNameLength];
            csfFile->read(reinterpret_cast<char*>(&labelName), sizeof(labelName));

            // Write string name with a special method due to string name doesn't have \0 sign
            QString stringName = QString::fromStdString(CharArrayToString(sizeof(labelName), reinterpret_cast<const char*>(labelName)));
            QString stringValue;
            QString extraStringValue;

            // There possible situation where exists empty strings
            if(countOfStrings != 0)
            {
                // Read string type
                uint8_t  rtsOrWrts[4];
                csfFile->read(reinterpret_cast<char*>(&rtsOrWrts), sizeof(rtsOrWrts));

                // Read string lenght
                uint32_t valueLenght;
                csfFile->read(reinterpret_cast<char*>(&valueLenght), sizeof(valueLenght));

                // Read byte-like compiled string
                wchar_t wchBufferValue[valueLenght];
                csfFile->read(reinterpret_cast<char*>(&wchBufferValue), sizeof(wchBufferValue));

                // Reverse read string
                for (int tmp = 0; tmp < valueLenght; tmp++)
                    wchBufferValue[tmp] = ~wchBufferValue[tmp];

                // Write string name with a special method due to string name doesn't have \0 sign
                stringValue = QString::fromStdWString(WharArrayToWstring(valueLenght, reinterpret_cast<const wchar_t*>(wchBufferValue)));

                // Read extra value and do not write bcs it's useless
                if((char)rtsOrWrts[0] == 'W')
                {
                    uint32_t extraValueLength;
                    csfFile->read(reinterpret_cast<char*>(&extraValueLength), sizeof(extraValueLength));

                    uint8_t extraValue[extraValueLength];
                    csfFile->read(reinterpret_cast<char*>(&extraValue), sizeof(extraValue));
                }

                // Add CompiledString{string Name, wstring Value} to list
                Table.push_back({stringName, stringValue});
            }
        }
    }

    void CSFParser::Save(const char*    strFileName) { wstring tmp = Path; Path = QString(strFileName).toStdWString();                Save(); Path = tmp; }
    void CSFParser::Save(const wstring& strFileName) { wstring tmp = Path; Path = strFileName;                                        Save(); Path = tmp; }
    void CSFParser::Save(const string&  strFileName) { wstring tmp = Path; Path = QString::fromStdString(strFileName).toStdWString(); Save(); Path = tmp; }
    void CSFParser::Save(const QString& strFileName) { wstring tmp = Path; Path = strFileName.toStdWString();                         Save(); Path = tmp; }
    void CSFParser::Save()
    {
        ofstream csfFile{Path.c_str(), ios::binary | ios::out};

        if(csfFile.is_open())
        {
            LOGSTM << (StringExt::EmptyString + "Attempt to write binary file \"" + Path + "\"").toStdString() << endl;

            CSFParser::WriteHeader(&csfFile);
            CSFParser::WriteBody(&csfFile);

            LOGSTM << (StringExt::EmptyString + "File saved as \"" + Path + "\"").toStdString() << endl;
        }
        else
        {
            LOGSTM << (StringExt::EmptyString + "Could not open file \"" + Path + "\" to save").toStdString() << endl;
        }

        csfFile.close();
    }

    void CSFParser::WriteHeader(ofstream* csfFile)
    {
        // Struct Header has the same length as the original file
        csfFile->write(reinterpret_cast<char*>(&Header), sizeof(Header));
    }

    void CSFParser::WriteBody(ofstream* csfFile)
    {
        const uint32_t ONE_STRING = 1;

        // Write normal strings
        for(const auto& elem : Table)
        {
            string name   = elem.Name.toStdString();
            wstring value = elem.Value.toStdWString();

            uint32_t labelLength  = name.size();
            uint32_t valueLength  = value.size();
            char     labelName      [labelLength];
            wchar_t  valueInversed  [valueLength];
         
            for(uint32_t i = 0; i < labelLength; i++)
                labelName[i] = name[i];

            for(uint32_t i = 0; i < valueLength; i++)
                valueInversed[i] = ~value[i];

            csfFile->write(reinterpret_cast<const char*>(LBL), sizeof(LBL));
            csfFile->write(reinterpret_cast<const char*>(&ONE_STRING), sizeof(ONE_STRING));
            csfFile->write(reinterpret_cast<char*>(&labelLength), sizeof(labelLength));
            csfFile->write(reinterpret_cast<char*>(&labelName), sizeof(labelName));

            csfFile->write(reinterpret_cast<const char*>(RTS), sizeof(RTS));
            csfFile->write(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
            csfFile->write(reinterpret_cast<char*>(&valueInversed), sizeof(valueInversed));
       }
    }
#pragma endregion

#pragma region Getters
    QString CSFParser::GetStringValue(const char* strName)   const { return GetStringValue(QString{strName}); }
    wstring CSFParser::GetStringValue(const string& strName) const { return GetStringValue(QString::fromStdString(strName)).toStdWString(); }
    QString CSFParser::GetStringValue(const QString& strName) const
    {
        QString returnValue;
        auto tmp = strName.toUpper();

        for (const auto& elem : Table)
            if (elem.Name.toUpper() == tmp)
            {
                returnValue = elem.Value;
                break;
            }

        return returnValue;
    }

    list<string> CSFParser::GetStringNames() const
    {
        list<string> returnList;
       
        for (const auto& elem : Table)
            returnList.push_back(elem.Name.toStdString());

        return returnList;
    }

    list<string> CSFParser::GetCategories() const
    {
        list<string> returnList;

        for (const auto& elem : Table)
            returnList.push_back(elem.Name.toStdString().substr(0, elem.Name.toStdString().find_first_of(':', 0) ));

        returnList.sort();
        returnList.unique();
        return returnList;
    }

    list<string> CSFParser::GetCategoryStrings(const string& strCategoryName) const
    {
        list<string> returnList;

        for (const auto& elem : Table)
            if (elem.Name.toStdString().substr(0, elem.Name.toStdString().find_first_of(':', 0)) == strCategoryName)
                returnList.push_back(elem.Name.toStdString().substr(elem.Name.toStdString().find_first_of(':', 0) + 1, elem.Name.size() - 1));

        returnList.sort();
        return returnList;
    }

    QStringList CSFParser::GetCategoryStrings(const QString& strCategoryName) const
    {
        list<string> tmp = GetCategoryStrings(strCategoryName.toStdString());
        return Convert::ToQStringList(tmp);
    }

    list<string> CSFParser::GetCategoryStringsWithFullNames(const string& strCategoryName) const
    {
        list<string> returnList;

        for (const auto& elem : Table)
            if (elem.Name.toStdString().substr(0, elem.Name.toStdString().find_first_of(':', 0)) == strCategoryName)
                returnList.push_back(elem.Name.toStdString());

        returnList.sort();
        return returnList;
    }

    QStringList CSFParser::GetCategoryStringsWithFullNames(const QString& strCategoryName) const
    {
        list<string> tmp = GetCategoryStringsWithFullNames(strCategoryName.toStdString());
        return Convert::ToQStringList(tmp);
    }
    
    list<string> CSFParser::GetStringsContainsSymbol(const wchar_t& wch) const
    {
        list<string> returnList;
       
        for (const auto& elem : Table)
            if (elem.Value.toStdWString().find(wch) <= elem.Value.size())
                returnList.push_back(elem.Name.toStdString());

        returnList.sort();
        return returnList;
    }

    list<string> CSFParser::GetStringsContainsSymbol(const wchar_t& wch, const string& strCategoryName) const
    {
        list<string> returnList;

        for (const auto& elem : Table)
            if (elem.Name.toStdString().substr(0, elem.Name.toStdString().find_first_of(':', 0)) == strCategoryName)
                if (elem.Value.toStdWString().find_first_of(wch) <= elem.Value.size())
                    returnList.push_back(elem.Name.toStdString());

        returnList.sort();
        return returnList;
    }

    list<CSFParser::CompiledString> CSFParser::GetStringsByNameList(const list<string>& lstNames) const
    {
        list<CompiledString> returnList;

        for (const auto& strName : lstNames)
            for (const auto& elem : Table)
                if (elem.Name == QString::fromStdString(strName))
                {
                    returnList.push_back(elem);
                    break;
                }

        return returnList;
    }

    QString CSFParser::GetClearName(const QString& strName)   const { return GetStringValue(strName).remove(QRegExp{"((\\[|\\()&([A-Z]|[a-z])(\\]|\\)))|&"}).trimmed(); }
    QChar   CSFParser::GetHotkey(const string& strName)       const { return QChar(GetHotkeyWchar(strName)); }
    QChar   CSFParser::GetHotkey(const char* strName)         const { return QChar(GetHotkeyWchar(strName)); }
    QChar   CSFParser::GetHotkey(const QString& strName)      const { return QChar(GetHotkeyWchar(strName)); }
    wchar_t CSFParser::GetHotkeyWchar(const char* strName)    const { return GetHotkeyWchar(string(strName)); }
    wchar_t CSFParser::GetHotkeyWchar(const string& strName)  const { return GetHotkeyWchar(QString::fromStdString(strName)); }
    wchar_t CSFParser::GetHotkeyWchar(const QString& strName) const
    {
        wchar_t hk    = L' ';
        size_t  index = 0;

        for (const auto& elem : Table)
            if (elem.Name.toUpper() == strName.trimmed().toUpper())
            {
                index = elem.Value.indexOf(L'&');
                if (index != -1 )
                {
                    hk = elem.Value.toUpper().toStdWString().at(index + 1);
                    break;
                }
            }

        if (hk == L' ')
        {
            LOGMSG("Warning: unable to find hotkey character for " + strName);
        }
        
        return hk;
    }

    list<CSFParser::HotkeyAssociation> CSFParser::GetHotkeys(const list<string>& lstStringNames) const
    {
        list<HotkeyAssociation> returnList;

        for (const auto& strName : lstStringNames)
            for (const auto& elem : Table)
                if (elem.Name == QString::fromStdString(strName))
                {
                    returnList.push_back({QString::fromStdString(strName), CSFParser::GetHotkeyWchar(strName)});
                    break;
                }

        return returnList;
    }
#pragma endregion

#pragma region Setters
    void CSFParser::SetHotkey(const string& input, const wchar_t& wchLetter) { SetHotkey(QString::fromStdString(input), wchLetter); }
    void CSFParser::SetHotkey(const char* strName, const wchar_t& wchLetter) { SetHotkey(string(strName), wchLetter); }
    void CSFParser::SetHotkey(const QString& strName, const wchar_t& wchLetter)
    {
        if (wchLetter == L' ') return;

        LOGSTM << "Changing for string \"" << strName.toStdString() << "\" hotkey assingment to letter \"" << (const char)wchLetter << "\"" << endl;

        for (auto& elem : Table)
        {
            if (elem.Name != strName)
                continue;

            if (!elem.Value.contains(L'&') || elem.Value.size() <= 4) // if not "[&F]"
                elem.Value = "[&"q + QChar(wchLetter) + "] " + elem.Value;
            else
                elem.Value[elem.Value.indexOf(L'&') + 1] = wchLetter;

            break;
        }
    }

    void CSFParser::SetStringValue(const string& strName, const wstring& wstrValue)
    {
        LOGSTM << "Changing value for string \"" << strName << "\"" << endl;

        QString name = QString::fromStdString(strName);
        for (auto& elem : Table)
            if (elem.Name == name)
                elem.Value = QString::fromStdWString(wstrValue);
    }

    void CSFParser::SetStringValue(const CompiledString& stString)
    {
        LOGSTM << "Changing value for string \"" << stString.Name.toStdString() << "\"" << endl;

        for (auto& elem : Table)
            if (elem.Name == stString.Name)
                elem.Value = stString.Value;
    }

    void CSFParser::SetStringsValue(const list<CompiledString>& lstChanges)
    {
        for (const auto& elem : lstChanges)
            SetStringValue(elem);
    }
#pragma endregion

#pragma region Checkers
    const bool CSFParser::ExistString(const char* value)        const { return ExistString(QString(value)); }
    const bool CSFParser::ExistString(const std::string& value) const { return ExistString(QString::fromStdString(value)); }
    const bool CSFParser::ExistString(const QString& value)     const 
    {
        QString valueUpper = value.toUpper();

        for (const auto& elem : Table)
            if (elem.Name.toUpper() == valueUpper)
                return true;

        return false;
    }

    const bool CSFParser::ExistCategory(const char* value)        const { return ExistCategory(QString(value)); }
    const bool CSFParser::ExistCategory(const std::string& value) const { return ExistCategory(QString::fromStdString(value)); }
    const bool CSFParser::ExistCategory(const QString& value)     const 
    {
        QString searchValue = (value + ":").toUpper();

        for (const auto& elem : Table)
            if (elem.Name.toUpper().startsWith(searchValue))
                return true;

        return false;
    }
#pragma endregion

#pragma region Internal methods
    string CSFParser::CharArrayToString(const size_t& arrayLength, const char* pArray) const
    {
        stringstream ss;
       
        for(size_t i = 0 ; i < arrayLength; i++)
            ss << pArray[i];
    
        return ss.str();
    }
    
    wstring CSFParser::WharArrayToWstring(const size_t& arrayLength, const wchar_t* pArray) const
    {
        wstringstream wss;
       
        for(size_t i = 0 ; i < arrayLength; i++)
            wss << pArray[i];
    
        return wss.str();
    }
#pragma endregion
