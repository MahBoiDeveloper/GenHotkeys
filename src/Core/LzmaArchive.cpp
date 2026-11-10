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

#include "LzmaArchive.hpp"
#include "../Extensions/ExceptionExt.hpp"

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
            ? QStringLiteral("%1 (SDK code 0x%2)").arg(operation).arg(quint32(result), 8, 16, QLatin1Char('0'))
            : operation + QStringLiteral(": ") + error);
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
            error = QString::fromUtf8(exception.what());
        }
        catch (...)
        {
            error = QStringLiteral("Unexpected archive callback error");
        }
        return E_FAIL;
    }

    QString AbsolutePath(const QString& path)
    {
        Require(!path.trimmed().isEmpty(), QStringLiteral("Path must not be empty"));
        return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    }

    void CheckNoLinks(QString path)
    {
        // Check ancestors as well: an ordinary file can live below a junction.
        for (;;)
        {
            const QFileInfo info(path);
            QString nativePath = QDir::toNativeSeparators(path);
            if (!nativePath.startsWith(QStringLiteral("\\\\?\\")))
                nativePath = nativePath.startsWith(QStringLiteral("\\\\"))
                    ? QStringLiteral("\\\\?\\UNC\\") + nativePath.mid(2)
                    : QStringLiteral("\\\\?\\") + nativePath;
            const DWORD attributes = GetFileAttributesW(reinterpret_cast<LPCWSTR>(nativePath.utf16()));
            Require(!info.isSymLink() && (attributes == INVALID_FILE_ATTRIBUTES
                || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT)),
                QStringLiteral("Links and reparse points are not supported: %1").arg(path));
            const QString parent = info.absolutePath();
            if (parent == path)
                break;
            path = parent;
        }
    }

    QString SafeRelativePath(QString path)
    {
        path.replace(QLatin1Char('\\'), QLatin1Char('/'));
        Require(!path.isEmpty() && !path.startsWith(QLatin1Char('/')),
            QStringLiteral("Invalid archive path: %1").arg(path));
        static const QRegularExpression reserved(
            QStringLiteral("^(CON|PRN|AUX|NUL|COM[1-9\u00b9\u00b2\u00b3]|LPT[1-9\u00b9\u00b2\u00b3])(?:\\.|$)"),
            QRegularExpression::CaseInsensitiveOption);
        for (const QString& part : path.split(QLatin1Char('/')))
        {
            Require(!part.isEmpty() && part != QStringLiteral(".") && part != QStringLiteral("..")
                && !part.endsWith(QLatin1Char('.')) && !part.endsWith(QLatin1Char(' '))
                && !reserved.match(part).hasMatch(), QStringLiteral("Unsafe archive path: %1").arg(path));
            for (const QChar character : part)
                Require(character.unicode() >= 32 && !QStringLiteral(":*?\"<>|").contains(character),
                    QStringLiteral("Unsafe archive path: %1").arg(path));
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
            Require(file.open(QIODevice::ReadOnly), QStringLiteral("Cannot read %1: %2").arg(path, file.errorString()));
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
        Require(info.exists() && (info.isFile() || info.isDir()), QStringLiteral("Invalid source: %1").arg(path));
        Require(items.size() < (std::numeric_limits<int>::max)(), QStringLiteral("Too many archive entries"));
        items.append({path, SafeRelativePath(name), info.isDir(), info.isDir() ? 0 : UInt64(info.size())});
        if (info.isDir())
        {
            Require(info.isReadable(), QStringLiteral("Cannot read directory: %1").arg(path));
            const auto entries = QDir(path).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name);
            for (const QFileInfo& entry : entries)
                CollectItems(entry.absoluteFilePath(), name + QLatin1Char('/') + entry.fileName(), items);
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
            Require(result == NArchive::NUpdate::NOperationResult::kOK, QStringLiteral("Compression failed"));
        });
    }

    QVector<Item> ReadItems(IInArchive* archive, const QString& destination)
    {
        UInt32 count = 0;
        CheckResult(archive->GetNumberOfItems(&count), QStringLiteral("Cannot read archive entries"));
        Require(count <= UInt32((std::numeric_limits<int>::max)()), QStringLiteral("Too many archive entries"));
        QVector<Item> items;
        QSet<QString> names;
        for (UInt32 index = 0; index < count; ++index)
        {
            CPropVariant path, directory, attributes, anti, encrypted;
            CheckResult(archive->GetProperty(index, kpidPath, &path), QStringLiteral("Cannot read entry path"));
            CheckResult(archive->GetProperty(index, kpidIsDir, &directory), QStringLiteral("Cannot read entry type"));
            CheckResult(archive->GetProperty(index, kpidAttrib, &attributes), QStringLiteral("Cannot read entry attributes"));
            CheckResult(archive->GetProperty(index, kpidIsAnti, &anti), QStringLiteral("Cannot read entry flags"));
            CheckResult(archive->GetProperty(index, kpidEncrypted, &encrypted), QStringLiteral("Cannot read entry encryption flag"));
            Require(path.vt == VT_BSTR && directory.vt == VT_BOOL, QStringLiteral("Invalid archive entry"));
            Require(anti.vt == VT_EMPTY || (anti.vt == VT_BOOL && anti.boolVal == VARIANT_FALSE), QStringLiteral("Anti-items are not supported"));
            Require(attributes.vt == VT_EMPTY || (attributes.vt == VT_UI4
                && !(attributes.ulVal & FILE_ATTRIBUTE_REPARSE_POINT)
                && !MY_LIN_S_ISLNK(attributes.ulVal >> 16)), QStringLiteral("Archive links are not supported"));
            Require(encrypted.vt == VT_EMPTY || (encrypted.vt == VT_BOOL && encrypted.boolVal == VARIANT_FALSE),
                QStringLiteral("Encrypted archives are not supported"));
            const QString name = SafeRelativePath(QString::fromWCharArray(path.bstrVal, int(SysStringLen(path.bstrVal))));
            const QString key = name.toCaseFolded();
            Require(!names.contains(key), QStringLiteral("Duplicate archive path: %1").arg(name));
            names.insert(key);
            const QString target = QDir(destination).filePath(name);
            CheckNoLinks(target);
            const bool isDirectory = directory.boolVal != VARIANT_FALSE;
            const QFileInfo existing(target);
            Require(!existing.exists() || (isDirectory && existing.isDir()), QStringLiteral("Destination already exists: %1").arg(target));
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
            while (parent.contains(QLatin1Char('/')))
            {
                parent.truncate(parent.lastIndexOf(QLatin1Char('/')));
                Require(!files.contains(parent.toCaseFolded()), QStringLiteral("Conflicting archive path: %1").arg(item.name));
                const QFileInfo info(QDir(destination).filePath(parent));
                Require(!info.exists() || info.isDir(), QStringLiteral("Destination directory is a file: %1").arg(info.filePath()));
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
            Require(!file.isOpen(), QStringLiteral("Previous archive entry was not finalized"));
            if (mode != NArchive::NExtract::NAskMode::kExtract)
                return;
            const Item& item = items.at(int(index));
            CheckNoLinks(item.path);
            Require(QDir().mkpath(item.directory ? item.path : QFileInfo(item.path).absolutePath()),
                QStringLiteral("Cannot create directory for %1").arg(item.path));
            if (!item.directory)
            {
                Require(!QFileInfo::exists(item.path), QStringLiteral("Destination already exists: %1").arg(item.path));
                file.setFileName(item.path);
                if (!file.open(QIODevice::WriteOnly))
                    throw ExceptionExt(QStringLiteral("Cannot write %1: %2").arg(item.path, file.errorString()));
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
                QStringLiteral("Archive integrity check failed (code %1)").arg(result));
            if (file.isOpen())
            {
                CheckNoLinks(file.fileName());
                Require(!QFileInfo::exists(file.fileName()), QStringLiteral("Destination already exists: %1").arg(file.fileName()));
                if (!file.commit())
                    throw ExceptionExt(QStringLiteral("Cannot commit extracted file: %1").arg(file.errorString()));
            }
        });
    }
}

void LzmaArchive::Pack(const QString& sourcePath, const QString& archivePath, int level)
{
    Require(level >= 0 && level <= 9, QStringLiteral("Compression level must be between 0 and 9"));
    const QString source = AbsolutePath(sourcePath);
    const QString target = AbsolutePath(archivePath);
    CheckNoLinks(source);
    CheckNoLinks(target);
    const QFileInfo sourceInfo(source);
    Require(sourceInfo.exists(), QStringLiteral("Source does not exist: %1").arg(source));
    Require(!QFileInfo::exists(target), QStringLiteral("Archive already exists: %1").arg(target));
    Require(source.compare(target, Qt::CaseInsensitive) != 0
        && (!sourceInfo.isDir() || !target.startsWith(source + QLatin1Char('/'), Qt::CaseInsensitive)),
        QStringLiteral("Archive must be outside the source directory"));

    QVector<Item> items;
    if (sourceInfo.isDir())
    {
        Require(sourceInfo.isReadable(), QStringLiteral("Cannot read directory: %1").arg(source));
        const auto entries = QDir(source).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::Name);
        for (const QFileInfo& entry : entries)
            CollectItems(entry.absoluteFilePath(), entry.fileName(), items);
    }
    else
        CollectItems(source, sourceInfo.fileName(), items);

    Require(QFileInfo(target).dir().exists(), QStringLiteral("Archive output directory does not exist"));
    QTemporaryFile temporary(QFileInfo(target).absolutePath() + QStringLiteral("/.genhotkeys-XXXXXX"));
    Require(temporary.open(), QStringLiteral("Cannot create archive: %1").arg(temporary.errorString()));
    CMyComPtr<IOutStream> output = new OutputStream(temporary);
    CMyComPtr<IOutArchive> archive = new NArchive::N7z::CHandler;
    CMyComPtr<ISetProperties> properties;
    CheckResult(archive.QueryInterface(IID_ISetProperties, &properties), QStringLiteral("Cannot configure archive"));
    const wchar_t* property = L"x";
    CPropVariant value;
    value = UInt32(level);
    CheckResult(properties->SetProperties(&property, &value, 1), QStringLiteral("Cannot set compression level"));
    CMyComPtr2<IArchiveUpdateCallback, PackCallback> callback;
    callback.SetFromCls(new PackCallback(items));
    const HRESULT result = archive->UpdateItems(output, UInt32(items.size()), callback);
    CheckResult(result, QStringLiteral("Cannot pack archive"), callback->Error);
    output.Release();
    Require(temporary.flush(), QStringLiteral("Cannot flush archive: %1").arg(temporary.errorString()));
    temporary.close();
    if (!temporary.rename(target))
        throw ExceptionExt(QStringLiteral("Cannot save archive %1: %2").arg(target, temporary.errorString()));
    temporary.setAutoRemove(false);
}

void LzmaArchive::Unpack(const QString& archivePath, const QString& destinationPath)
{
    const QString source = AbsolutePath(archivePath);
    const QString destination = AbsolutePath(destinationPath);
    CheckNoLinks(destination);
    Require(!QFileInfo::exists(destination) || QFileInfo(destination).isDir(), QStringLiteral("Destination must be a directory"));
    CMyComPtr<IInStream> input = new InputStream(source);
    CMyComPtr<IInArchive> archive = new NArchive::N7z::CHandler;
    CheckResult(archive->Open(input, nullptr, nullptr), QStringLiteral("Cannot open 7z archive"));
    const QVector<Item> items = ReadItems(archive, destination);
    Require(QDir().mkpath(destination), QStringLiteral("Cannot create destination directory: %1").arg(destination));
    CMyComPtr2<IArchiveExtractCallback, UnpackCallback> callback;
    callback.SetFromCls(new UnpackCallback(items));
    const HRESULT result = archive->Extract(nullptr, UInt32(-1), false, callback);
    CheckResult(result, QStringLiteral("Cannot unpack archive"), callback->Error);
    CheckResult(archive->Close(), QStringLiteral("Cannot close archive"));
}
