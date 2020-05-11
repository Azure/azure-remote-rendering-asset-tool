#pragma once
#include <Model/IncludesAzureStorage.h>
#include <QObject>
#include <QPointer>
#include <Utils/Value.h>

class QStandardItemModel;
class AzureStorageManager;

// class representing a directory path, notifying when it changes

class DirectoryProvider : public QObject
{
    Q_OBJECT
public:
    DirectoryProvider(AzureStorageManager* storageManager, Value<QString>* container, const QString& directory, QObject* parent = {});

    void goToParentDirectory();
    bool canGoToParentDirectory();

    QString getDirectory() const;
    // set directory. It only works when the directory is valid. Valid directories end with "/"
    bool setDirectory(const QString& directory);

    // append the directory to the current directory and navigate to it
    void navigateIntoDirectory(const QString& directory);

    const utility::string_t& getPrefix() const;
    utility::string_t getRelativePath(const utility::string_t& absolutePath) const;

    bool isBlobInDirectory(const utility::string_t& blobPath) const;

    QPointer<QAbstractItemModel> createNextDirectoriesModel(const QString& fromDirectory);

Q_SIGNALS:
    void directoryChanged();

private:
    AzureStorageManager* const m_storageManager;
    QString m_directory;
    utility::string_t m_prefix;
    QPointer<Value<QString>> m_container;
};
