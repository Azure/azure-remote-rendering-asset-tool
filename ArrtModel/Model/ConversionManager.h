#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <Model/IncludesAzureStorage.h>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QTime>
#include <QTimer>
#include <shared_mutex>

class AzureStorageManager;
class ArrFrontend;

// struct representing an conversion. It holds the current status and the parameters used (input/output)

struct Conversion
{
    // Parameters below holds the data for a RR::StartAssetConversionAsync (where the strings are const char*)

    // taken from arr_asset_conversion_input_sas_params
    std::string m_input_storage_account_name;
    std::string m_input_blob_container_name;
    std::string m_input_folder;
    std::string m_input_asset_relative_path;
    azure::storage::storage_uri m_inputContainer;

    // taken from arr_asset_conversion_output_sas_params
    std::string m_output_storage_account_name;
    std::string m_output_blob_container_name;
    std::string m_output_folder;
    std::string m_output_asset_relative_path;
    azure::storage::storage_uri m_outputContainer;

    std::string m_renderingSettings;
    std::string m_materialOverrides;

    // optional: this indicates the local directory we are synchronizing from
    std::wstring m_synchronizeFromLocalDir;

    // status of the conversion
    enum Status
    {
        UNKNOWN,
        NOT_STARTED,
        START_REQUESTED,
        FAILED_TO_START,
        STARTING,
        SYNCHRONIZING,
        CONVERTING,
        COMPLETED,
        CANCELED,
        SYNCHRONIZATION_FAILED,
        CONVERSION_FAILED
    } m_status = NOT_STARTED;

    std::string m_activeSessionUUID;
    // stop/watch timer used to time the conversion
    QTime m_startConversionTime;
    QTime m_endConversionTime;

    QString m_name;

    bool isActive() const { return m_status == START_REQUESTED || m_status == STARTING || m_status == SYNCHRONIZING || m_status == CONVERTING; }
    bool isFinished() const { return m_status == FAILED_TO_START || m_status == COMPLETED || m_status == CANCELED || m_status == SYNCHRONIZATION_FAILED || m_status == CONVERSION_FAILED || m_status == UNKNOWN; }

    void updateConversionStatus(Status newStatus, const QString& message = QString());

    std::string getInputModelFullPath() const
    {
        return m_input_folder + m_input_asset_relative_path;
    }

    std::string getInputModelDirectory() const
    {
        std::string dir = getInputModelFullPath();
        const auto lastSlash = dir.find_last_of('/');
        if (lastSlash == std::string::npos)
        {
            return {};
        }
        else
        {
            return dir.substr(0, lastSlash + 1);
        }
    }

    bool changeRootDirectory(std::string directory)
    {
        std::string fullPath = getInputModelFullPath();
        if (directory != m_input_folder && fullPath._Starts_with(directory))
        {
            m_input_folder = std::move(directory);
            m_input_asset_relative_path = fullPath.substr(m_input_folder.length());
            return true;
        }
        else
        {
            return false;
        }
    }

    // return the name of the model
    QString getModelName() const
    {
        return QString::fromStdString(m_input_asset_relative_path).section('/', -1).section('.', 0, -2);
	}

    // return the default name that would be given to this conversion if m_name is not set. Id is the conversion id, which might be used in the default name
    QString getDefaultName(int id) const
    {
        const QString name = getModelName();
        if (!name.isEmpty())
        {
            return name;
        }
        else
        {
            return QCoreApplication::tr("Conversion %1").arg(id);
        }
    }

    RR::ApiHandle<RR::ConversionStatusAsync> m_statusAsync = nullptr;
    RR::ApiHandle<RR::StartAssetConversionAsync> m_conversionCall = nullptr;
};

// class used to control conversions in ARRT. It will keep track of the current conversions and the past ones, and allow the user to start new ones or
// cancel/remove them

class ConversionManager : public QObject
{
    Q_OBJECT
public:
    typedef uint ConversionId;

    ConversionManager(ArrFrontend* client, AzureStorageManager* storageManager, QObject* parent = nullptr);
    ~ConversionManager();

    int getConversionsCount() const;
    ConversionId getConversionId(int idx) const;

    const Conversion* getConversion(ConversionId id) const;
    Conversion* getConversion(ConversionId id);

    ConversionId addNewConversion();

    // the blob provider is used to generate the Sas tokens
    void startConversion(ConversionId id, const AzureStorageManager* storageManager);
    void removeConversion(ConversionId id);

    void setConversionName(ConversionId id, const QString& name);

    bool isEnabled() const;

    // return the number of running conversions
    int runningConversionCount() const;

    static const QString s_default_input_container;
    static const QString s_default_output_container;

Q_SIGNALS:
    void onEnabledChanged();
    void conversionUpdated(ConversionId id);
    void conversionAdded(ConversionId id);
    void conversionRemoved(ConversionId id);
    void runningConversionCountChanged();
    void conversionCompleted(ConversionId id, bool success);

private:
    ArrFrontend* const m_frontend;
    // this is only needed to generate SAS from the input urls. <TODO> see if it can be removed
    AzureStorageManager* const m_storageManager;

    // map from conversion ID to conversion data
    QMap<ConversionId, Conversion*> m_conversions;

    ConversionId m_highestId = 0;

    QTimer* m_updateTimer = nullptr;

    bool m_enabled = false;

    int m_runningConversionCount = 0;

    int m_secondsUntilNextUpdate = 0;

    //update all of the active conversions
    void updateConversions(bool updateRemotely);

    void changeConversionCount(int delta);
};
