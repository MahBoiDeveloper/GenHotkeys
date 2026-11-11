#include <limits>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTemporaryFile>
#include <QVector>

#include "../../libs/lzma-sdk/CPP/Common/Common.h"
#include "../../libs/lzma-sdk/CPP/Common/MyInitGuid.h"
#include "../../libs/lzma-sdk/CPP/Common/MyLinux.h"
#include "../../libs/lzma-sdk/CPP/Windows/PropVariant.h"
#include "../../libs/lzma-sdk/CPP/7zip/Archive/7z/7zHandler.h"

#include "../Extensions/ExceptionExt.hpp"
#include "../Extensions/StringExt.hpp"
#include "LzmaArchive.hpp"

using namespace StringExt;

#pragma region LZMA SDK wrapper

namespace
{
    using NWindows::NCOM::CPropVariant;

    void Require(bool condition, const QString& message)
    {
        if (!condition)
            throw ExceptionExt(message);
    }

    void CheckResult(HRESULT result, const QString& operation, const QString& error = {})
    {
        Require(result == S_OK, error.isEmpty()
            ? operation + " (SDK code 0x%1)"q.arg(quint32(result), 8, 16, QLatin1Char('0'))
            : operation + ": "q + error);
    }

    // COM callbacks must return errors to the SDK, without letting exceptions escape.
    template<class Function>
    HRESULT Callback(QString& error, Function function) noexcept
    {
        try
        {
            function();
            return S_OK;
        }
        catch (const std::exception& exception)
        {
            error = ToQString(exception.what());
        }
        catch (...)
        {
            error = "Unexpected archive callback error"q;
        }
        return E_FAIL;
    }

    QString AbsolutePath(const QString& path)
    {
        Require(!path.trimmed().isEmpty(), "Path must not be empty"q);
        return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    }

    void CheckNoLinks(QString path)
    {
        // Check ancestors as well: an ordinary file can live below a junction.
        for (;;)
        {
            const QFileInfo info(path);
            QString nativePath = QDir::toNativeSeparators(path);
            if (!nativePath.startsWith("\\\\?\\"q))
                nativePath = nativePath.startsWith("\\\\"q)
                    ? "\\\\?\\UNC\\"q + nativePath.mid(2)
                    : "\\\\?\\"q + nativePath;
            const DWORD attributes = GetFileAttributesW(reinterpret_cast<LPCWSTR>(nativePath.utf16()));
            Require(!info.isSymLink() && (attributes == INVALID_FILE_ATTRIBUTES
                || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT)),
                "Links and reparse points are not supported: "q + path);
            const QString parent = info.absolutePath();
            if (parent == path)
                break;
            path = parent;
        }
    }

    QString SafeRelativePath(QString path)
    {
        path.replace('\\', '/');
        Require(!path.isEmpty() && !path.startsWith('/'),
            "Invalid archive path: "q + path);
        static const QRegularExpression reserved(
            "^(CON|PRN|AUX|NUL|COM[1-9\u00b9\u00b2\u00b3]|LPT[1-9\u00b9\u00b2\u00b3])(?:\\.|$)"q,
            QRegularExpression::CaseInsensitiveOption);
        for (const QString& part : path.split('/'))
        {
            Require(!part.isEmpty() && part != "."q && part != ".."q
                && !part.endsWith('.') && !part.endsWith(' ')
                && !reserved.match(part).hasMatch(), "Unsafe archive path: "q + path);
            for (const QChar character : part)
                Require(character.unicode() >= 32 && !":*?\"<>|"q.contains(character),
                    "Unsafe archive path: "q + path);
        }
        return path;
    }

    HRESULT SeekFile(QFileDevice& file, Int64 offset, UInt32 origin, UInt64* position)
    {
        if (origin > STREAM_SEEK_END)
            return STG_E_INVALIDFUNCTION;
        const qint64 base = origin == STREAM_SEEK_SET ? 0
            : origin == STREAM_SEEK_CUR ? file.pos() : file.size();
        if (base < 0 || offset < -base || offset > (std::numeric_limits<qint64>::max)() - base)
            return STG_E_INVALIDFUNCTION;
        if (!file.seek(base + offset))
            return E_FAIL;
        if (position)
            *position = UInt64(file.pos());
        return S_OK;
    }

    class InputStream final : public IInStream, public CMyUnknownImp
    {
        Z7_IFACES_IMP_UNK_2(ISequentialInStream, IInStream)
        QFile file;
    public:
        explicit InputStream(const QString& path) : file(path)
        {
            CheckNoLinks(path);
            Require(file.open(QIODevice::ReadOnly), "Cannot read "q + path + ": "q + file.errorString());
        }
    };

    Z7_COM7F_IMF(InputStream::Read(void* data, UInt32 size, UInt32* processedSize))
    {
        if (processedSize)
            *processedSize = 0;
        const qint64 count = file.read(static_cast<char*>(data), size);
        if (count < 0)
            return E_FAIL;
        if (processedSize)
            *processedSize = UInt32(count);
        return S_OK;
    }

    Z7_COM7F_IMF(InputStream::Seek(Int64 offset, UInt32 origin, UInt64* position))
    {
        return SeekFile(file, offset, origin, position);
    }

    class OutputStream final : public IOutStream, public CMyUnknownImp
    {
        Z7_IFACES_IMP_UNK_2(ISequentialOutStream, IOutStream)
        QFileDevice& file;
    public:
        explicit OutputStream(QFileDevice& output) : file(output) {}
    };

    Z7_COM7F_IMF(OutputStream::Write(const void* data, UInt32 size, UInt32* processedSize))
    {
        if (processedSize)
            *processedSize = 0;
        const qint64 count = file.write(static_cast<const char*>(data), size);
        if (count < 0 || (size != 0 && count == 0))
            return E_FAIL;
        if (processedSize)
            *processedSize = UInt32(count);
        return S_OK;
    }

    Z7_COM7F_IMF(OutputStream::Seek(Int64 offset, UInt32 origin, UInt64* position))
    {
        return SeekFile(file, offset, origin, position);
    }

    Z7_COM7F_IMF(OutputStream::SetSize(UInt64 size))
    {
        return size <= UInt64((std::numeric_limits<qint64>::max)()) && file.resize(qint64(size)) ? S_OK : E_FAIL;
    }

    struct Item
    {
        QString path;
        QString name;
        bool directory;
        UInt64 size;
    };

    void CollectItems(const QString& path, const QString& name, QVector<Item>& items)
    {
        CheckNoLinks(path);
        const QFileInfo info(path);
        Require(info.exists() && (info.isFile() || info.isDir()), "Invalid source: "q + path);
        Require(items.size() < (std::numeric_limits<int>::max)(), "Too many archive entries"q);
        items.append({path, SafeRelativePath(name), info.isDir(), info.isDir() ? 0 : UInt64(info.size())});
        if (info.isDir())
        {
            Require(info.isReadable(), "Cannot read directory: "q + path);
            const auto entries = QDir(path).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name);
            for (const QFileInfo& entry : entries)
                CollectItems(entry.absoluteFilePath(), name + '/' + entry.fileName(), items);
        }
    }

    class PackCallback final : public IArchiveUpdateCallback, public CMyUnknownImp
    {
        Z7_IFACES_IMP_UNK_2(IProgress, IArchiveUpdateCallback)
        const QVector<Item>& items;
    public:
        QString Error;
        explicit PackCallback(const QVector<Item>& sourceItems) : items(sourceItems) {}
    };

    Z7_COM7F_IMF(PackCallback::SetTotal(UInt64)) { return S_OK; }
    Z7_COM7F_IMF(PackCallback::SetCompleted(const UInt64*)) { return S_OK; }

    Z7_COM7F_IMF(PackCallback::GetUpdateItemInfo(UInt32, Int32* newData, Int32* newProperties, UInt32* indexInArchive))
    {
        if (newData) *newData = 1;
        if (newProperties) *newProperties = 1;
        if (indexInArchive) *indexInArchive = UInt32(-1);
        return S_OK;
    }

    Z7_COM7F_IMF(PackCallback::GetProperty(UInt32 index, PROPID property, PROPVARIANT* value))
    {
        return Callback(Error, [&]
        {
            const Item& item = items.at(int(index));
            CPropVariant result;
            switch (property)
            {
                case kpidPath: result = item.name.toStdWString().c_str(); break;
                case kpidIsDir: result = item.directory; break;
                case kpidSize: result = item.size; break;
                case kpidIsAnti: result = false; break;
            }
            result.Detach(value);
        });
    }

    Z7_COM7F_IMF(PackCallback::GetStream(UInt32 index, ISequentialInStream** stream))
    {
        *stream = nullptr;
        return Callback(Error, [&]
        {
            const Item& item = items.at(int(index));
            if (!item.directory)
            {
                CMyComPtr<ISequentialInStream> input = new InputStream(item.path);
                *stream = input.Detach();
            }
        });
    }

    Z7_COM7F_IMF(PackCallback::SetOperationResult(Int32 result))
    {
        return Callback(Error, [&]
        {
            Require(result == NArchive::NUpdate::NOperationResult::kOK, "Compression failed"q);
        });
    }

    QVector<Item> ReadItems(IInArchive* archive, const QString& destination)
    {
        UInt32 count = 0;
        CheckResult(archive->GetNumberOfItems(&count), "Cannot read archive entries"q);
        Require(count <= UInt32((std::numeric_limits<int>::max)()), "Too many archive entries"q);
        QVector<Item> items;
        QSet<QString> names;
        for (UInt32 index = 0; index < count; ++index)
        {
            CPropVariant path, directory, attributes, anti, encrypted;
            CheckResult(archive->GetProperty(index, kpidPath, &path),           "Cannot read entry path"q);
            CheckResult(archive->GetProperty(index, kpidIsDir, &directory),     "Cannot read entry type"q);
            CheckResult(archive->GetProperty(index, kpidAttrib, &attributes),   "Cannot read entry attributes"q);
            CheckResult(archive->GetProperty(index, kpidIsAnti, &anti),         "Cannot read entry flags"q);
            CheckResult(archive->GetProperty(index, kpidEncrypted, &encrypted), "Cannot read entry encryption flag"q);
            Require(path.vt == VT_BSTR && directory.vt == VT_BOOL, "Invalid archive entry"q);
            Require(anti.vt == VT_EMPTY || (anti.vt == VT_BOOL && anti.boolVal == VARIANT_FALSE), "Anti-items are not supported"q);
            Require(attributes.vt == VT_EMPTY || (attributes.vt == VT_UI4
                && !(attributes.ulVal & FILE_ATTRIBUTE_REPARSE_POINT)
                && !MY_LIN_S_ISLNK(attributes.ulVal >> 16)), "Archive links are not supported"q);
            Require(encrypted.vt == VT_EMPTY || (encrypted.vt == VT_BOOL && encrypted.boolVal == VARIANT_FALSE),
                "Encrypted archives are not supported"q);
            const QString name = SafeRelativePath(QString::fromWCharArray(path.bstrVal, int(SysStringLen(path.bstrVal))));
            const QString key = name.toCaseFolded();
            Require(!names.contains(key), "Duplicate archive path: "q + name);
            names.insert(key);
            const QString target = QDir(destination).filePath(name);
            CheckNoLinks(target);
            const bool isDirectory = directory.boolVal != VARIANT_FALSE;
            const QFileInfo existing(target);
            Require(!existing.exists() || (isDirectory && existing.isDir()), "Destination already exists: "q + target);
            items.append({target, name, isDirectory, 0});
        }
        // Reject file/directory conflicts before writing any entry.
        QSet<QString> files;
        for (const Item& item : items)
            if (!item.directory)
                files.insert(item.name.toCaseFolded());
        for (const Item& item : items)
        {
            QString parent = item.name;
            while (parent.contains('/'))
            {
                parent.truncate(parent.lastIndexOf('/'));
                Require(!files.contains(parent.toCaseFolded()), "Conflicting archive path: "q + item.name);
                const QFileInfo info(QDir(destination).filePath(parent));
                Require(!info.exists() || info.isDir(), "Destination directory is a file: "q + info.filePath());
            }
        }
        return items;
    }

    class UnpackCallback final : public IArchiveExtractCallback, public CMyUnknownImp
    {
        Z7_IFACES_IMP_UNK_2(IProgress, IArchiveExtractCallback)
        const QVector<Item>& items;
        QSaveFile file;
        CMyComPtr<ISequentialOutStream> output;
    public:
        QString Error;
        explicit UnpackCallback(const QVector<Item>& archiveItems) : items(archiveItems) {}
    };

    Z7_COM7F_IMF(UnpackCallback::SetTotal(UInt64)) { return S_OK; }
    Z7_COM7F_IMF(UnpackCallback::SetCompleted(const UInt64*)) { return S_OK; }
    Z7_COM7F_IMF(UnpackCallback::PrepareOperation(Int32)) { return S_OK; }

    Z7_COM7F_IMF(UnpackCallback::GetStream(UInt32 index, ISequentialOutStream** stream, Int32 mode))
    {
        *stream = nullptr;
        return Callback(Error, [&]
        {
            output.Release();
            Require(!file.isOpen(), "Previous archive entry was not finalized"q);
            if (mode != NArchive::NExtract::NAskMode::kExtract)
                return;
            const Item& item = items.at(int(index));
            CheckNoLinks(item.path);
            Require(QDir().mkpath(item.directory ? item.path : QFileInfo(item.path).absolutePath()),
                "Cannot create directory for "q + item.path);
            if (!item.directory)
            {
                Require(!QFileInfo::exists(item.path), "Destination already exists: "q + item.path);
                file.setFileName(item.path);
                if (!file.open(QIODevice::WriteOnly))
                    throw ExceptionExt("Cannot write "q + item.path + ": "q + file.errorString());
                output = new OutputStream(file);
                *stream = output;
                (*stream)->AddRef();
            }
        });
    }

    Z7_COM7F_IMF(UnpackCallback::SetOperationResult(Int32 result))
    {
        return Callback(Error, [&]
        {
            output.Release();
            Require(result == NArchive::NExtract::NOperationResult::kOK,
                "Archive integrity check failed (code "q + result + ')');
            if (file.isOpen())
            {
                CheckNoLinks(file.fileName());
                Require(!QFileInfo::exists(file.fileName()), "Destination already exists: "q + file.fileName());
                if (!file.commit())
                    throw ExceptionExt("Cannot commit extracted file: "q + file.errorString());
            }
        });
    }
}

#pragma endregion

void LzmaArchive::Pack(const QString& sourcePath, const QString& archivePath, int level)
{
    Require(level >= 0 && level <= 9, "Compression level must be between 0 and 9"q);
    const QString source = AbsolutePath(sourcePath);
    const QString target = AbsolutePath(archivePath);
    CheckNoLinks(source);
    CheckNoLinks(target);
    const QFileInfo sourceInfo(source);
    Require(sourceInfo.exists(), "Source does not exist: "q + source);
    Require(!QFileInfo::exists(target), "Archive already exists: "q + target);
    Require(source.compare(target, Qt::CaseInsensitive) != 0
        && (!sourceInfo.isDir() || !target.startsWith(source + '/', Qt::CaseInsensitive)),
        "Archive must be outside the source directory"q);

    QVector<Item> items;
    if (sourceInfo.isDir())
    {
        Require(sourceInfo.isReadable(), "Cannot read directory: "q + source);
        const auto entries = QDir(source).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name);
        for (const QFileInfo& entry : entries)
            CollectItems(entry.absoluteFilePath(), entry.fileName(), items);
    }
    else
        CollectItems(source, sourceInfo.fileName(), items);

    Require(QFileInfo(target).dir().exists(), "Archive output directory does not exist"q);
    QTemporaryFile temporary(QFileInfo(target).absolutePath() + "/.genhotkeys-XXXXXX"q);
    Require(temporary.open(), "Cannot create archive: "q + temporary.errorString());
    CMyComPtr<IOutStream> output = new OutputStream(temporary);
    CMyComPtr<IOutArchive> archive = new NArchive::N7z::CHandler;
    CMyComPtr<ISetProperties> properties;
    CheckResult(archive.QueryInterface(IID_ISetProperties, &properties), "Cannot configure archive"q);
    const wchar_t* property = L"x";
    CPropVariant value;
    value = UInt32(level);
    CheckResult(properties->SetProperties(&property, &value, 1), "Cannot set compression level"q);
    CMyComPtr2<IArchiveUpdateCallback, PackCallback> callback;
    callback.SetFromCls(new PackCallback(items));
    const HRESULT result = archive->UpdateItems(output, UInt32(items.size()), callback);
    CheckResult(result, "Cannot pack archive"q, callback->Error);
    output.Release();
    Require(temporary.flush(), "Cannot flush archive: "q + temporary.errorString());
    
    temporary.close();
    if (!temporary.rename(target))
        throw ExceptionExt("Cannot save archive "q + target + ": "q + temporary.errorString());
    temporary.setAutoRemove(false);
}

void LzmaArchive::Unpack(const QString& archivePath, const QString& destinationPath)
{
    const QString source      = AbsolutePath(archivePath);
    const QString destination = AbsolutePath(destinationPath);

    CheckNoLinks(destination);
    Require(!QFileInfo::exists(destination) || QFileInfo(destination).isDir(), "Destination must be a directory"q);

    CMyComPtr<IInStream> input = new InputStream(source);
    CMyComPtr<IInArchive> archive = new NArchive::N7z::CHandler;

    CheckResult(archive->Open(input, nullptr, nullptr), "Cannot open 7z archive"q);
    const QVector<Item> items = ReadItems(archive, destination);
    Require(QDir().mkpath(destination), "Cannot create destination directory: "q + destination);
    CMyComPtr2<IArchiveExtractCallback, UnpackCallback> callback;
    callback.SetFromCls(new UnpackCallback(items));
    const HRESULT result = archive->Extract(nullptr, UInt32(-1), false, callback);

    CheckResult(result, "Cannot unpack archive"q, callback->Error);
    CheckResult(archive->Close(), "Cannot close archive"q);
}
