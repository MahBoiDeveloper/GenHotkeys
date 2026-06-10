#pragma once
#include <QFileInfo>
#include "../Core/ProgramConstants.hpp"

class ImageManager
{
private: // Data
    inline static QMap<QString, QImage> ImagesCache;
public:

private: // Methods
    /// @brief Search recursively icons in folders.
    static QFileInfo FindIconFile(const QString& pathToIconsDir, const QString& fileBaseName);
    /// @brief Decode WEBP image from path to the icon.
    static QImage    DecodeWebpIconPath(const QString& iconPath);
    /// @brief Decode WEBP image from byte array.
    static QImage    DecodeImageFromData(const QByteArray& iconData);
public:
    ImageManager() = delete;

    /// @brief Decode WEBP image icon name.
    static QImage  DecodeWebpIcon(const QString& iconName);
    /// @brief Get image if needed file cannot be found.
    static QImage  DecodeMissingWebpIcon();
    /// @brief Return image of the main executable WEBP icon.
    static QImage  DecodeEditorWebpIcon();
    /// @brief Return project's official WEBP image.
    static QImage  DecodeBigEditorWebpIcon();
    /// @brief Return pixel map by object type.
    static QPixmap GetGameObjectTypePixmap(GameObjectTypes goType);
};
