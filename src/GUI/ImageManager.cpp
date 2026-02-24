#include <QDir>
#include <QImage>
#include <QPixmap>

#include "../../libs/libwebp/src/webp/decode.h"
#include "../Logger.hpp"
#include "ImageManager.hpp"

QImage ImageManager::DecodeWebpIcon(const QString& iconName)
{
    // If it there, get the image from cache
    const auto it = ImagesCache.constFind(iconName);
    if (it != ImagesCache.constEnd()) return it.value();

    // Find
    const QFileInfo targetIconFile = FindIconFile(PROGRAM_CONSTANTS->ICONS_FOLDER, iconName);

    if (targetIconFile.exists())
    {
        // Get the image
        const QImage targetImage = DecodeWebpIconPath(targetIconFile.absoluteFilePath());
        // Save the image in the cache
        ImagesCache.insert(iconName, targetImage);
        // Return the image
        return targetImage;
    }
    else
    {
        return DecodeMissingWebpIcon();
    }
}

QImage ImageManager::DecodeMissingWebpIcon()   { return DecodeWebpIconPath(PROGRAM_CONSTANTS->MISSING_ICON_FILE); }
QImage ImageManager::DecodeEditorWebpIcon()    { return DecodeWebpIconPath(PROGRAM_CONSTANTS->EDITOR_ICON_FILE); }
QImage ImageManager::DecodeBigEditorWebpIcon() { return DecodeWebpIconPath(PROGRAM_CONSTANTS->EDITOR_BIG_ICON_FILE); }

QPixmap ImageManager::GetGameObjectTypePixmap(GameObjectTypes entityType)
{
    return QPixmap{PROGRAM_CONSTANTS->QT_ICONS_FOLDER + "/" + PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS.value(entityType) + ".webp"};
}

QFileInfo ImageManager::FindIconFile(const QString& pathToIconsDir, const QString& fileBaseName)
{
    // Find all files and dirs in current directory
    QFileInfoList fileInfoList = QDir{pathToIconsDir}.entryInfoList(QDir::Filter::Files |
                                                                    QDir::Filter::Dirs  |
                                                                    QDir::Filter::NoDotAndDotDot);

    for (const auto & fileInfo : fileInfoList)
    {
        // if dir -> recursive find
        if (fileInfo.isDir())
        {
            QFileInfo foundFile = FindIconFile(fileInfo.absoluteFilePath(), fileBaseName);

            // Return if it's a complete match
            if (foundFile.baseName() == fileBaseName) return foundFile;
        }
        // else if it is a matching file -> return it
        else
        {
            if (fileInfo.baseName() == fileBaseName) return fileInfo;
        }
    }

    return QFileInfo{};
}

QImage ImageManager::DecodeWebpIconPath(const QString& iconPath)
{
    QFile iconFile(iconPath);

    if(iconPath.isEmpty() || !iconFile.open(QIODevice::ReadOnly))
    {
        if (!iconPath.isEmpty())
        {
            LOGMSG("No icon file [" + iconFile.fileName() + "] was found");
        }
        return DecodeMissingWebpIcon();
    }

    return DecodeImageFromData(iconFile.readAll());
}

QImage ImageManager::DecodeImageFromData(const QByteArray& iconData)
{
    int width, height;
    uint8_t* decodedImage = WebPDecodeRGBA(reinterpret_cast<const uint8_t*>(iconData.constData()), // amogus
                                           iconData.size(),
                                           &width,
                                           &height);
    return QImage{decodedImage, width, height, QImage::Format_RGBA8888};
}
