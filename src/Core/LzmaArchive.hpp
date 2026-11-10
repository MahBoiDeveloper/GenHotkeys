#pragma once
#include <QString>

/// @brief Reads and creates .7z archives using the bundled LZMA SDK.
class LzmaArchive final
{
public:
    LzmaArchive() = delete;

    /// @brief Packs one file or the contents of a directory, including empty directories.
    /// Compression level is 0 (store) through 9; the default is 5 (LZMA2).
    /// The archive must not exist and must be outside the source directory.
    /// Symbolic links and Windows reparse points are rejected.
    /// @throws ExceptionExt on invalid input, SDK errors or filesystem errors.
    static void Pack(const QString& sourcePath, const QString& archivePath, int level = 5);

    /// @brief Extracts a .7z archive, creating the destination directory as needed.
    /// Existing files, unsafe paths, links and encrypted archives are rejected.
    /// Each file is committed only after its integrity check passes. An error may
    /// leave directories and files already extracted successfully.
    /// @throws ExceptionExt on invalid input, SDK errors or filesystem errors.
    static void Unpack(const QString& archivePath, const QString& destinationPath);
};
