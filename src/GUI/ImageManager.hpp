#pragma once
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include "../Core/ProgramConstants.hpp"

class ImageManager
{
private: // Data
    inline static QMap<QPair<QString, QString>, QImage> ImagesCache;

private: // Methods
    /// @brief Find an icon recursively, preferring WEBP, PNG, JPG, then JPEG.
    static QFileInfo FindIconFile(const QString& pathToIconsDir, const QString& fileBaseName);
    /// @brief Read an image, returning a null image on failure.
    static QImage LoadImageFromPath(const QString& iconPath);
public:
    ImageManager() = delete;

    /// @brief Load a WEBP, PNG, JPG and other supported by Qt profile icon by name without extension.
    static QImage  LoadIcon(const QString& iconName);
    /// @brief Get the placeholder for missing or unreadable icons.
    static QImage  LoadMissingIcon();
    /// @brief Return the main executable icon.
    static QImage  LoadEditorIcon();
    /// @brief Return the project's official image.
    static QImage  LoadBigEditorIcon();
    /// @brief Return pixel map by object type.
    static QPixmap GetGameObjectTypePixmap(GameObjectTypes goType);
};
