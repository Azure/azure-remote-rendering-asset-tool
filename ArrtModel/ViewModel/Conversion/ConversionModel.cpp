#include <Model/AzureStorageManager.h>
#include <Model/ConversionManager.h>
#include <Model/Log/LogHelpers.h>
#include <QPointer>
#include <QStandardItemModel>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/Conversion/ConversionConfigModel.h>
#include <ViewModel/Conversion/ConversionModel.h>
#include <ViewModel/Conversion/CurrentConversionsModel.h>
#include <ViewModel/Conversion/InputSelectionModel.h>
#include <ViewModel/Conversion/OutputSelectionModel.h>

namespace
{
    QStandardItem* newDirectoryItem(const QString& dir)
    {
        QStandardItem* item = new QStandardItem(dir.isEmpty() ? QCoreApplication::tr("[root]") : dir);
        item->setEditable(false);
        item->setData(dir, Qt::UserRole);
        return item;
    }
} // namespace


ConversionModel::ConversionModel(ConversionManager* conversionManager, AzureStorageManager* storageManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_conversionManager(conversionManager)
    , m_storageManager(storageManager)
    , m_conversionConfigModel(new ConversionConfigModel(storageManager, this))
    , m_rootDirectoryModel(new QStandardItemModel(this))
    , m_configuration(configuration)
{
    QObject::connect(m_conversionManager, &ConversionManager::conversionUpdated, this,
                     [this](int id) {
                         if (m_conversionId == id)
                         {
                             changed();
                         }
                     });
}

ConversionModel::~ConversionModel()
{
    m_conversionConfigModel->save();
}

void ConversionModel::setConversion(int conversionId)
{
    m_conversionId = conversionId;

    loadConfigFileForConversion(getConversion());
    updateRootDirectoryModel();
    changed();
}

QString ConversionModel::getDefaultName() const
{
    if (const Conversion* conversion = getConversion())
    {
        return conversion->getDefaultName(m_conversionId);
    }
    return {};
}

QString ConversionModel::getName() const
{
    if (const Conversion* conversion = getConversion())
    {
        return conversion->m_name;
    }
    return {};
}

void ConversionModel::setName(const QString& name)
{
    m_conversionManager->setConversionName(m_conversionId, name);
}

bool ConversionModel::canSetName() const
{
    return getConversion() != nullptr;
}

QString ConversionModel::getStatus() const
{
    if (const Conversion* conversion = getConversion())
    {
        return CurrentConversionsModel::getStringFromStatus(conversion->m_status);
    }
    return {};
}

QString ConversionModel::getInput() const
{
    if (const Conversion* conversion = getConversion())
    {
        if (!conversion->m_synchronizeFromLocalDir.empty())
        {
            return QString("file://") + QString::fromStdWString(conversion->m_synchronizeFromLocalDir);
        }
        else if (!conversion->m_inputContainer.primary_uri().is_empty())
        {
            return QString::fromStdWString(
                       conversion->m_inputContainer.primary_uri().to_string()) +
                   "/" + QString::fromUtf8(conversion->m_input_folder.c_str()) + QString::fromUtf8(conversion->m_input_asset_relative_path.c_str());
        }
        else
        {
            return {};
        }
    }
    else
    {
        return {};
    }
}

// return the model that can be used to select the root directory
QAbstractItemModel* ConversionModel::getInputRootDirectorySelectorModel() const
{
    return m_rootDirectoryModel;
}

QString ConversionModel::getCurrentInputRootDirectory() const
{
    if (const Conversion* conversion = getConversion())
    {
        return QString::fromStdString(conversion->m_input_folder);
    }
    return {};
}

void ConversionModel::setCurrentInputRootDirectory(const QString& currentRootDirectory)
{
    if (!m_inhibitCurrentInputRootDirectory)
    {
        if (Conversion* conversion = m_conversionManager->getConversion(m_conversionId))
        {
            if (conversion->changeRootDirectory(currentRootDirectory.toStdString()))
            {
                Q_EMIT changed();
            }
        }
    }
}

QString ConversionModel::getOutput() const
{
    if (const Conversion* conversion = getConversion())
    {
        if (!conversion->m_outputContainer.primary_uri().is_empty())
        {
            return QString::fromStdWString(conversion->m_outputContainer.primary_uri().to_string()) + "/" + QString::fromUtf8(conversion->m_output_folder.c_str()) + conversion->getModelName() + ".arrAsset";
        }
        else
        {
            return {};
        }
    }
    else
    {
        return {};
    }
}

bool ConversionModel::canSelectInput() const
{
    if (const Conversion* conversion = getConversion())
    {
        return !conversion->isActive();
    }
    else
    {
        return false;
    }
}

InputSelectionModel* ConversionModel::createtInputSelectionModel()
{
    auto* conversion = getConversion();
    if (!conversion)
    {
        return nullptr;
    }
    const QString container = QString::fromStdString(conversion->m_input_blob_container_name);
    const QString directory = QString::fromStdString(conversion->getInputModelDirectory());
    auto* const model = new InputSelectionModel(m_storageManager, container, directory, m_configuration, this);

    model->getExplorerModel()->setSelectedBlob(
        QString::fromStdString(conversion->m_input_asset_relative_path),
        QString::fromStdString(conversion->m_input_folder + conversion->m_input_asset_relative_path));

    const int conversionId = m_conversionId;
    QObject::connect(model, &InputSelectionModel::submitted, this, [this, model, conversionId]() {
        if (conversionId != m_conversionId)
        {
            return;
        }
        if (Conversion* conversion = m_conversionManager->getConversion(conversionId))
        {
            QString sourceContainer = model->getContainersModel()->getCurrentContainer();
            QString sourceDirectory = model->getExplorerModel()->getDirectory();
            QString sourceRelativePath = model->getExplorerModel()->getSelectedRelativeBlobPath();
            Conversion& info = *conversion;
            info.m_input_storage_account_name = QString::fromStdWString(m_storageManager->getAccountName()).toStdString();
            info.m_input_blob_container_name = sourceContainer.toStdString();
            info.m_input_folder = sourceDirectory.toStdString();
            info.m_input_asset_relative_path = sourceRelativePath.toStdString();
            info.m_inputContainer = m_storageManager->getContainerUriFromName(sourceContainer);
            // this will change the root directory to be one where the model is.
            info.changeRootDirectory(info.getInputModelDirectory());

            loadConfigFileForConversion(conversion);
            updateRootDirectoryModel();
            changed();
        }
    });
    return model;
}

bool ConversionModel::canSelectOutput() const
{
    return canSelectInput();
}

OutputSelectionModel* ConversionModel::createOutputSelectionModel()
{
    auto* conversion = getConversion();
    if (!conversion)
    {
        return nullptr;
    }
    const QString container = QString::fromStdString(conversion->m_output_blob_container_name);
    const QString directory = QString::fromStdString(conversion->m_output_folder);

    auto* const model = new OutputSelectionModel(m_storageManager, container, directory, m_configuration, this);
    const int conversionId = m_conversionId;
    QObject::connect(model, &OutputSelectionModel::submitted, this, [this, model, conversionId]() {
        if (Conversion* conversion = m_conversionManager->getConversion(conversionId))
        {
            QString destContainer = model->getContainersModel()->getCurrentContainer();
            QString destDirectory = model->getExplorerModel()->getSelectedBlobPath();
            if (destDirectory.isEmpty())
            {
                destDirectory = model->getExplorerModel()->getDirectory();
            }
            Conversion& info = *conversion;
            info.m_output_storage_account_name = QString::fromStdWString(m_storageManager->getAccountName()).toStdString();
            info.m_output_blob_container_name = destContainer.toStdString();
            info.m_output_folder = destDirectory.toStdString();
            info.m_output_asset_relative_path = "";
            info.m_outputContainer = m_storageManager->getContainerUriFromName(destContainer);
            changed();
        }
    });


    return model;
}

bool ConversionModel::canStartConversion() const
{
    if (const Conversion* conversion = getConversion())
    {
        return !conversion->isActive() && !conversion->m_input_asset_relative_path.empty();
    }
    else
    {
        return false;
    }
}

void ConversionModel::startConversion()
{
    if (canStartConversion())
    {
        Conversion* conversion = m_conversionManager->getConversion(m_conversionId);
        conversion->updateConversionStatus(Conversion::START_REQUESTED);

        if (m_conversionConfigModel->save())
        {
            // try and create the output container, if it's not there

            QPointer<ConversionModel> thisPtr = this;
            m_storageManager->createContainer(QString::fromStdString(conversion->m_output_blob_container_name), [thisPtr, conversionId = m_conversionId](bool success) {
                if (thisPtr)
                {
                    if (success)
                    {
                        thisPtr->m_conversionManager->startConversion(conversionId, thisPtr->m_storageManager);
                    }
                    else
                    {
                        Conversion* conversion = thisPtr->m_conversionManager->getConversion(conversionId);
                        if (conversion)
                        {
                            conversion->updateConversionStatus(Conversion::FAILED_TO_START);
						}
                        qCritical(LoggingCategory::conversion) << tr("Conversion couldn't start because output container could not be created");
                    }
                } });
        }
        else
        {
            conversion->updateConversionStatus(Conversion::FAILED_TO_START);
            qCritical(LoggingCategory::conversion) << tr("Conversion couldn't start because configuration couldn't be saved on blob storage");
        }
    }
}

bool ConversionModel::isConfigurationEnabled() const
{
    if (const Conversion* conversion = getConversion())
    {
        return (!conversion->isActive()) && (!conversion->m_input_asset_relative_path.empty());
    }
    return false;
}

bool ConversionModel::isConfigurationDefault() const
{
    return m_conversionConfigModel->isDefault();
}

void ConversionModel::resetToDefault()
{
    m_conversionConfigModel->restoreToDefault();
    changed();
}

const QList<ParameterModel*>& ConversionModel::getConfigurationControls() const
{
    return m_conversionConfigModel->getControls();
}

bool ConversionModel::isConversionSelected() const
{
    return m_conversionId != s_conversionNotSelected;
}

const Conversion* ConversionModel::getConversion() const
{
    return m_conversionManager->getConversion(m_conversionId);
}

void ConversionModel::loadConfigFileForConversion(const Conversion* conversion)
{
    m_conversionConfigModel->save();
    if (conversion)
    {
        if ((conversion->m_input_asset_relative_path.empty()))
        {
            m_conversionConfigModel->unload();
        }
        else
        {
            QString inputFolder = QString::fromStdString(conversion->getInputModelDirectory());
            m_conversionConfigModel->load(inputFolder.toStdWString(), conversion->m_inputContainer);
        }
    }
    else
    {
        m_conversionConfigModel->unload();
    }
}

void ConversionModel::updateRootDirectoryModel()
{
    if (const Conversion* conversion = getConversion())
    {
        QString dir = QString::fromStdString(conversion->getInputModelDirectory());

        if (m_lastModelDirectory == dir)
        {
            return;
        }
        m_lastModelDirectory = dir;

        m_inhibitCurrentInputRootDirectory = true;
        m_rootDirectoryModel->clear();

        while (!dir.isEmpty())
        {
            m_rootDirectoryModel->insertRow(0, newDirectoryItem(dir));

            const int lastIndex = dir.lastIndexOf('/', -2) + 1;
            dir = dir.left(lastIndex);
        }
        m_rootDirectoryModel->insertRow(0, newDirectoryItem({}));

        m_inhibitCurrentInputRootDirectory = false;
    }
    else
    {
        m_lastModelDirectory.clear();
        m_rootDirectoryModel->clear();
    }
}
