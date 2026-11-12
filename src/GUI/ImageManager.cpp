#include <QDir>
#include <QImageReader>
#include <QStringList>

#include "../Core/Logger.hpp"
#include "WindowManager.hpp"
#include "ImageManager.hpp"

QImage ImageManager::LoadActionIcon(const QString& iconName)
{
    const QString iconsDirectory = QDir{WINDOW_MANAGER->ProfileFolder + "/" + PROGRAM_CONSTANTS->ICONS_FOLDER_NAME}.absolutePath();
    const auto cacheKey = qMakePair(iconsDirectory, iconName);
    const auto it = ImagesCache.constFind(cacheKey);
    if (it != ImagesCache.constEnd()) return it.value();

    const QFileInfo targetIconFile = FindIconFile(iconsDirectory, iconName);
    QImage image;

    if (targetIconFile.exists())
    {
        image = LoadImageFromPath(targetIconFile.absoluteFilePath());
    }
    else
    {
        LOGMSG("Icon \"" + iconName + "\" was not found in \"" + iconsDirectory + "\"");
    }

    if (image.isNull()) image = LoadMissingIcon();
    ImagesCache.insert(cacheKey, image);
    return image;
}

QImage ImageManager::LoadMissingIcon()   { return LoadImageFromPath(PROGRAM_CONSTANTS->MISSING_ICON_FILE); }
QImage ImageManager::LoadEditorIcon()    { return LoadImageFromPath(PROGRAM_CONSTANTS->EDITOR_ICON_FILE); }
QImage ImageManager::LoadBigEditorIcon() { return LoadImageFromPath(PROGRAM_CONSTANTS->EDITOR_BIG_ICON_FILE); }
QPixmap ImageManager::GetGameObjectTypePixmap(GameObjectTypes entityType)
{
    return QPixmap{PROGRAM_CONSTANTS->QT_ICONS_FOLDER + "/" + PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS.value(entityType) + ".webp"};
}

QFileInfo ImageManager::FindIconFile(const QString& pathToIconsDir, const QString& fileBaseName)
{
    if (fileBaseName.isEmpty()) return QFileInfo{};

    // Discover image plugins only after QApplication has initialized their paths.
    const auto exts = QImageReader::supportedImageFormats();

    const QFileInfoList fileInfoList = QDir{pathToIconsDir}.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                                                                        QDir::Name | QDir::IgnoreCase);
    QFileInfo bestMatch;
    int bestPriority = exts.size();

    for (const auto& fileInfo : fileInfoList)
    {
        const QFileInfo candidate = fileInfo.isDir()
            ? FindIconFile(fileInfo.absoluteFilePath(), fileBaseName)
            : fileInfo;

        if (!candidate.isFile() || candidate.completeBaseName() != fileBaseName) continue;

        const int priority = exts.indexOf(candidate.suffix().toLower().toLocal8Bit());
        if (priority >= 0 && priority < bestPriority)
        {
            bestMatch = candidate;
            bestPriority = priority;
            if (priority == 0) break;
        }
    }

    return bestMatch;
}

QImage ImageManager::LoadImageFromPath(const QString& iconPath)
{
    QImageReader reader(iconPath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();

    if (image.isNull())
    {
        LOGMSG("Unable to load icon [" + iconPath + "]: " + reader.errorString());
    }

    return image;
}
