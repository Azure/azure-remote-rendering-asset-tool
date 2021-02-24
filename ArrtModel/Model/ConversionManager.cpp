#include <Model/ArrFrontend.h>
#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/ConversionManager.h>
#include <Model/Log/LogHelpers.h>
#include <QApplication>
#include <QDebug>
#include <QPointer>

const QString ConversionManager::s_default_input_container = "arrt-sourcemodels";
const QString ConversionManager::s_default_output_container = "arrt-convertedmodels";

namespace
{
    const int m_secondsForEachUpdate = 7;

    QString toString(const Conversion::Status& conversionStatus)
    {
        switch (conversionStatus)
        {
            case Conversion::NOT_STARTED:
                return QCoreApplication::tr("not_started");
            case Conversion::START_REQUESTED:
                return QCoreApplication::tr("start_requested");
            case Conversion::FAILED_TO_START:
                return QCoreApplication::tr("failed_to_start");
            case Conversion::STARTING:
                return QCoreApplication::tr("starting");
            case Conversion::SYNCHRONIZING:
                return QCoreApplication::tr("synchronizing");
            case Conversion::CONVERTING:
                return QCoreApplication::tr("converting");
            case Conversion::COMPLETED:
                return QCoreApplication::tr("completed");
            case Conversion::CANCELED:
                return QCoreApplication::tr("canceled");
            case Conversion::SYNCHRONIZATION_FAILED:
                return QCoreApplication::tr("synchronization_failed");
            case Conversion::CONVERSION_FAILED:
                return QCoreApplication::tr("conversion_failed");
            case Conversion::UNKNOWN:
                // nothing
                break;
        }
        return QCoreApplication::tr("unknown_status");
    }

    inline QDebug& operator<<(QDebug& logger, const Conversion& conversion)
    {
        QJsonObject info;
        info[QLatin1String("status")] = toString(conversion.m_status);
        info[QLatin1String("uuid")] = conversion.m_status >= Conversion::STARTING ? QString::fromStdString(conversion.m_conversionUUID) : QCoreApplication::tr("unknown");
        if (conversion.m_status >= Conversion::STARTING)
        {
            info[QLatin1String("start time")] = conversion.m_startConversionTime.toString();
        }
        if (conversion.m_status >= Conversion::COMPLETED)
        {
            info[QLatin1String("end time")] = conversion.m_endConversionTime.toString();
        }
        QJsonObject generalInfo;

        QJsonObject input_sas_params;
        input_sas_params[QLatin1String("input_storage_account_name")] = QString::fromStdString(conversion.m_input_storage_account_name);
        input_sas_params[QLatin1String("input_blob_container_name")] = QString::fromStdString(conversion.m_input_blob_container_name);
        input_sas_params[QLatin1String("input_folder")] = QString::fromStdString(conversion.m_input_folder);
        input_sas_params[QLatin1String("input_asset_relative_path")] = QString::fromStdString(conversion.m_input_asset_relative_path);
        input_sas_params[QLatin1String("inputContainer")] = QString::fromStdWString(conversion.m_inputContainer.path());
        generalInfo[QLatin1String("input_sas_params")] = input_sas_params;

        QJsonObject output_sas_params;
        output_sas_params[QLatin1String("output_storage_account_name")] = QString::fromStdString(conversion.m_output_storage_account_name);
        output_sas_params[QLatin1String("output_blob_container_name")] = QString::fromStdString(conversion.m_output_blob_container_name);
        output_sas_params[QLatin1String("output_folder")] = QString::fromStdString(conversion.m_output_folder);
        output_sas_params[QLatin1String("output_asset_relative_path")] = QString::fromStdString(conversion.m_output_asset_relative_path);
        output_sas_params[QLatin1String("outputContainer")] = QString::fromStdWString(conversion.m_outputContainer.path());
        generalInfo[QLatin1String("output_sas_params")] = output_sas_params;

        generalInfo[QLatin1String("renderingSettings")] = QString::fromStdString(conversion.m_renderingSettings);
        generalInfo[QLatin1String("materialOverrides")] = QString::fromStdString(conversion.m_materialOverrides);
        generalInfo[QLatin1String("synchronizeFromLocalDir")] = QString::fromStdWString(conversion.m_synchronizeFromLocalDir);
        info[QLatin1String("general")] = generalInfo;

        logger << PrettyJson(info);
        return logger;
    }

    void logContext(const RR::SessionGeneralContext& context)
    {
        if (context.Result != RR::Result::Success)
        {
            qWarning(LoggingCategory::conversion)
                << QCoreApplication::tr("Request failed:\n") << context;
        }
        else
        {
            qDebug(LoggingCategory::conversion)
                << QCoreApplication::tr("Request succeeded:\n") << context;
        }
    }

    void logConversionStatusUpdate(QDebug&& logger, const Conversion& conversion, Conversion::Status prevStatus, const QString& message)
    {
        logger
            << QCoreApplication::tr("Conversion status update:")
            << toString(prevStatus) << "->" << toString(conversion.m_status);
        if (!message.isEmpty())
        {
            logger << QCoreApplication::tr(". Message: ") << message;
        };
        logger << "\n"
               << conversion;
    }
} // namespace

void Conversion::updateConversionStatus(Conversion::Status newStatus, const QString& message)
{
    if (m_status != newStatus || !message.isEmpty())
    {
        const auto prevStatus = m_status;
        m_status = newStatus;
        if (m_status == CONVERSION_FAILED || m_status == FAILED_TO_START || m_status == SYNCHRONIZATION_FAILED)
        {
            logConversionStatusUpdate(qWarning(LoggingCategory::conversion), *this, prevStatus, message);
        }
        else
        {
            logConversionStatusUpdate(qInfo(LoggingCategory::conversion), *this, prevStatus, message);
        }
    }
}

ConversionManager::ConversionManager(ArrFrontend* azureFrontend, AzureStorageManager* storageManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_frontend(azureFrontend)
    , m_storageManager(storageManager)
    , m_configuration(configuration)
{
    auto onStatusChanged = [this]() {
        const bool enabled = m_frontend->getStatus() == AccountConnectionStatus::Authenticated &&
                             m_storageManager->getStatus() == AccountConnectionStatus::Authenticated;
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            Q_EMIT onEnabledChanged();
        }
    };
    connect(m_frontend, &ArrFrontend::onStatusChanged, this, onStatusChanged);
    connect(m_storageManager, &AzureStorageManager::onStatusChanged, this, onStatusChanged);

    m_updateTimer = new QTimer(this);

    auto onSecondTick = [this]() {
        --m_secondsUntilNextUpdate;
        if (m_secondsUntilNextUpdate <= 0)
        {
            updateConversions(true);
            m_secondsUntilNextUpdate = m_secondsForEachUpdate;
        }
        else
        {
            updateConversions(false);
        }
    };

    QObject::connect(m_updateTimer, &QTimer::timeout, this, onSecondTick);
    using namespace std::chrono_literals;
    m_updateTimer->setInterval(1s);
}

ConversionManager::~ConversionManager()
{
    for (auto it = m_conversions.begin(); it != m_conversions.end(); ++it)
    {
        delete it.value();
    }
}

int ConversionManager::getConversionsCount() const
{
    return m_conversions.size();
}

ConversionManager::ConversionId ConversionManager::getConversionId(int idx) const
{
    assert(idx >= 0 && idx < getConversionsCount());
    return (m_conversions.begin() + idx).key();
}

Conversion* ConversionManager::getConversion(ConversionId id)
{
    auto conversionIt = m_conversions.find(id);
    if (conversionIt == m_conversions.end())
    {
        return nullptr;
    }
    return *conversionIt;
}

const Conversion* ConversionManager::getConversion(ConversionId id) const
{
    auto conversionIt = m_conversions.find(id);
    if (conversionIt == m_conversions.end())
    {
        return nullptr;
    }
    return *conversionIt;
}

ConversionManager::ConversionId ConversionManager::addNewConversion()
{
    const ConversionId newConversionId = ++m_highestId;

    auto* newConversion = new Conversion();
    newConversion->m_output_storage_account_name = QString::fromStdWString(m_storageManager->getAccountName()).toStdString();

    const QString outputContainer = m_configuration->getUiState(QLatin1Literal("outputSelection:defaultContainer"), s_default_output_container);
    newConversion->m_output_blob_container_name = outputContainer.toStdString();
    newConversion->m_outputContainer = m_storageManager->getContainerUriFromName(outputContainer);
    newConversion->m_output_folder = m_configuration->getUiState(QLatin1Literal("outputSelection:defaultDirectory"), QString()).toStdString();

    m_conversions.insert(newConversionId, newConversion);

    Q_EMIT conversionAdded(newConversionId);
    return newConversionId;
}

void ConversionManager::startConversion(ConversionManager::ConversionId newConversionId, const AzureStorageManager* storageManager)
{
    Conversion* newConversion = getConversion(newConversionId);
    assert(newConversion);

    newConversion->updateConversionStatus(Conversion::START_REQUESTED);
    QPointer<ConversionManager> thisPtr = this;

    auto onConversionStartRequestFinished = [thisPtr, newConversionId](RR::Status status, RR::ApiHandle<RR::AssetConversionResult> result) {
        std::string conversionUUID;
        RR::Result errorCode = result->GetErrorCode();
        if (status == RR::Status::OK && errorCode == RR::Result::Success)
        {
            logContext(result->GetContext());
            result->GetConversionUuid(conversionUUID);
        }
        QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, newConversionId, errorCode, conversionUUID]() {
            if (thisPtr)
            {
                if (Conversion* conversion = thisPtr->getConversion(newConversionId))
                {
                    if (errorCode == RR::Result::Success)
                    {
                        conversion->m_conversionUUID = conversionUUID;
                        conversion->updateConversionStatus(Conversion::SYNCHRONIZING);
                    }
                    else
                    {
                        conversion->m_endConversionTime.start();
                        conversion->updateConversionStatus(Conversion::SYNCHRONIZATION_FAILED, tr("Failure reason: %1.").arg(RR::ResultToString(errorCode)));
                    }
                }
                Q_EMIT thisPtr->conversionUpdated(newConversionId);
            }
        });
    };

    QString inputSasToken = QString::fromStdWString(storageManager->getSasToken(newConversion->m_inputContainer,
                                                                                azure::storage::blob_shared_access_policy::read |
                                                                                    azure::storage::blob_shared_access_policy::list,
                                                                                60 * 24));
    QString outputSasToken = QString::fromStdWString(storageManager->getSasToken(newConversion->m_outputContainer,
                                                                                 azure::storage::blob_shared_access_policy::write |
                                                                                     azure::storage::blob_shared_access_policy::list |
                                                                                     azure::storage::blob_shared_access_policy::create,
                                                                                 60 * 24));

    QString inputUri, outputUri;
    inputUri.sprintf("https://%s.blob.core.windows.net/%s", newConversion->m_input_storage_account_name.c_str(), newConversion->m_input_blob_container_name.c_str());
    outputUri.sprintf("https://%s.blob.core.windows.net/%s", newConversion->m_output_storage_account_name.c_str(), newConversion->m_output_blob_container_name.c_str());

    RR::AssetConversionOptions options;

    RR::AssetConversionInputOptions& input(options.InputOptions);
    input.BlobPrefix = newConversion->m_input_folder;
    input.RelativeInputAssetPath = newConversion->m_input_asset_relative_path;
    input.StorageContainerReadListSas = inputSasToken.toStdString();
    input.StorageContainerUri = inputUri.toStdString();

    RR::AssetConversionOutputOptions& output(options.OutputOptions);
    output.BlobPrefix = newConversion->m_output_folder;
    output.OutputAssetFilename = newConversion->m_output_asset_relative_path;
    output.StorageContainerWriteSas = outputSasToken.toStdString();
    output.StorageContainerUri = outputUri.toStdString();

    m_frontend->getFrontend()->StartAssetConversionAsync(options, onConversionStartRequestFinished);
    newConversion->m_startConversionTime.start();
    newConversion->m_endConversionTime = newConversion->m_startConversionTime;
    thisPtr->changeConversionCount(1);
}

void ConversionManager::removeConversion(ConversionId id)
{
    auto it = m_conversions.find(id);
    if (it != m_conversions.end())
    {
        delete *it;
        m_conversions.erase(it);
        conversionRemoved(id);
    }
}

void ConversionManager::setConversionName(ConversionId id, const QString& name)
{
    if (Conversion* conversion = getConversion(id))
    {
        conversion->m_name = name;
        Q_EMIT conversionUpdated(id);
    }
}

bool ConversionManager::isEnabled() const
{
    return m_enabled;
}

int ConversionManager::runningConversionCount() const
{
    return m_runningConversionCount;
}

void ConversionManager::updateConversions(bool updateRemotely)
{
    for (auto it = m_conversions.begin(); it != m_conversions.end(); ++it)
    {
        Conversion* conversion = it.value();
        ConversionId id = it.key();

        // if it's in "START_REQUESTED" state it means the server hasn't answered yet, so we can't query the state.
        if (conversion->isActive() && conversion->m_status != Conversion::START_REQUESTED && !conversion->m_statusQueryInProgress)
        {
            conversion->m_endConversionTime.start();
            Q_EMIT conversionUpdated(id);
            if (updateRemotely)
            {
                //query conversion
                conversion->m_statusQueryInProgress = true;
                QPointer<ConversionManager> thisPtr = this;
                m_frontend->getFrontend()->GetAssetConversionStatusAsync(conversion->m_conversionUUID, [id, thisPtr](RR::Status status, RR::ApiHandle<RR::AssetConversionStatusResult> result) {
                    std::string message;
                    RR::ConversionSessionStatus conversionResult;
                    if (status == RR::Status::OK)
                    {
                        logContext(result->GetContext());
                        result->GetErrorMessage(message);
                        conversionResult = result->GetResult();
                    }
                    else
                    {
                        conversionResult = RR::ConversionSessionStatus::Failure;
                        message = "Failure";
                    }

                    QMetaObject::invokeMethod(QApplication::instance(), [id, thisPtr, message, status, conversionResult]() {
                        if (thisPtr)
                        {
                            if (Conversion* conversion = thisPtr->getConversion(id))
                            {
                                conversion->m_statusQueryInProgress = false;
                                if (status == RR::Status::OK)
                                {
                                    Conversion::Status conversionStatus = Conversion::UNKNOWN;
                                    switch (conversionResult)
                                    {
                                        case RR::ConversionSessionStatus::Created:
                                            conversionStatus = Conversion::STARTING;
                                            break;
                                        case RR::ConversionSessionStatus::Running:
                                            conversionStatus = Conversion::CONVERTING;
                                            break;
                                        case RR::ConversionSessionStatus::Aborted:
                                            conversionStatus = Conversion::CANCELED;
                                            break;
                                        case RR::ConversionSessionStatus::Failure:
                                            conversionStatus = Conversion::CONVERSION_FAILED;
                                            break;
                                        case RR::ConversionSessionStatus::Success:
                                            conversionStatus = Conversion::COMPLETED;
                                            break;
                                        case RR::ConversionSessionStatus::Unknown:
                                            conversionStatus = Conversion::UNKNOWN;
                                            break;
                                    }
                                    conversion->m_endConversionTime.start();

                                    const bool wasActive = conversion->isActive();
                                    conversion->updateConversionStatus(conversionStatus, QString::fromStdString(message));
                                    const bool isActive = conversion->isActive();
                                    if (wasActive != isActive)
                                    {
                                        thisPtr->changeConversionCount(isActive ? 1 : -1);
                                        if (!isActive)
                                        {
                                            Q_EMIT thisPtr->conversionCompleted(id, conversionStatus == Conversion::COMPLETED);
                                        }
                                    }
                                    Q_EMIT thisPtr->conversionUpdated(id);
                                }
                            }
                        }
                    });
                });
            }
        }
    }
}

void ConversionManager::changeConversionCount(int delta)
{
    const bool wasActive = m_runningConversionCount > 0;
    m_runningConversionCount += delta;
    const bool isActive = m_runningConversionCount > 0;

    assert(m_runningConversionCount >= 0);
    Q_EMIT runningConversionCountChanged();

    if (wasActive != isActive)
    {
        if (isActive)
        {
            m_secondsUntilNextUpdate = 0;
            m_updateTimer->start();
        }
        else
        {
            m_updateTimer->stop();
        }
    }
}
