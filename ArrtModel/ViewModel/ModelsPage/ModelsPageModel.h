#pragma once
#include <QObject>
#include <ViewModel/BlobExplorer/BlobsListModel.h>

class QAbstractItemModel;
class AzureStorageManager;
class ArrSessionManager;
class BlobContainerSelectorModel;
class BlobExplorerModel;
class Configuration;

// model for ModelsPageView, exposing the models for container selection and blob hierarchy inspection

class ModelsPageModel : public QObject
{
    Q_OBJECT

public:
    enum LoadingMode
    {
        FromExplorer,
        FromSasUri
    };

    ModelsPageModel(AzureStorageManager* storageManager, ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);
    virtual ~ModelsPageModel();

    BlobContainerSelectorModel* getContainersModel() const;
    BlobExplorerModel* getExplorerModel() const;

    void setDirectory(const QString& directory);
    QString getDirectory() const;

    QString getCurrentLoadingModel() const;
    BlobsListModel::LoadingStatus getCurrentLoadingStatus() const;
    float getCurrentLoadingProgress() const;

    QString getModelSasUri() const;
    void setModelSasUri(QString sasUri);

    bool isEnabled() const;
    bool isBlobStorageAvailable() const;

    // refresh container list and blob list
    void refresh();

    // true when load can be invoked
    bool canLoad(LoadingMode mode) const;
    // start loading the selected model
    void load(LoadingMode mode);

Q_SIGNALS:
    // notify when a model has been loaded
    void modelLoaded(bool loaded);
    void loadingStatusChanged();

    void onEnabledChanged();
    void onBlobStorageAvailabilityChanged();
    void canLoadChanged(LoadingMode mode);

private:
    AzureStorageManager* const m_storageManager;
    Configuration* const m_configuration;
    BlobContainerSelectorModel* const m_containersModel;
    BlobExplorerModel* const m_explorerModel;
    ArrSessionManager* const m_sessionManager;

    QString m_modelSasUri;

    QString m_currentModel;
    BlobsListModel::LoadingStatus m_loadingStatus = BlobsListModel::LoadingStatus::NOT_LOADED;
    float m_loadingProgress = 0.0f;
    bool m_loadingFromExplorer = false;

    bool m_blobStorageAvailable = false;

    bool m_canLoad = false;
    bool m_enabled = false;

    bool loadModelImpl(const QString& path, const QString& sasUri, bool fromExplorer);

    void setCurrentLoadingModel(const QString& model, bool fromExplorer);
    void setCurrentLoadingStatus(BlobsListModel::LoadingStatus status);
    void setCurrentLoadingProgress(float progress);
};
