#pragma once

#include <Model/ConversionManager.h>
#include <QAction>
#include <QObject>

class QContainerSelectorModel;
class ConversionManager;
class InputSelectionModel;
class OutputSelectionModel;
class ConversionConfigModel;
class AzureStorageManager;
class ParameterModel;
class QAbstractItemModel;
class QStandardItemModel;
class Configuration;
struct Conversion;

// model for ConversionView which represents a single conversion (new conversion or past conversion)

class ConversionModel : public QObject
{
    Q_OBJECT

public:
    ConversionModel(ConversionManager* conversionManager, AzureStorageManager* storageManager, Configuration* configuration, QObject* parent);
    ~ConversionModel();
    void setConversion(int conversionId);

    // return the automatic name the conversion is assigned, in case the name is not set
    QString getDefaultName() const;

    // return the user-defined name of the conversion
    QString getName() const;

    // set the user-defined name
    void setName(const QString& name);

    // true if setName can be called
    bool canSetName() const;

    // return the current status of the conversion
    QString getStatus() const;

    QString getInput() const;

    // return the model that can be used to select the root directory
    QAbstractItemModel* getInputRootDirectorySelectorModel() const;

    QString getCurrentInputRootDirectory() const;
    void setCurrentInputRootDirectory(const QString& currentRootDirectory);

    QString getOutput() const;

    bool canSelectInput() const;
    InputSelectionModel* createtInputSelectionModel();

    bool canSelectOutput() const;
    OutputSelectionModel* createOutputSelectionModel();

    bool canStartConversion() const;
    void startConversion();

    // return true if the user can modify the configuration
    bool isConfigurationEnabled() const;

    // return true if the configuration is the default configuration
    bool isConfigurationDefault() const;

    // reset the configuration to default values
    void resetToDefault();

    // return all of the viewmodels for the configuration parameters
    const QList<ParameterModel*>& getConfigurationControls() const;

    // return false when there is no selected conversion in the model, which should cause the UI to remove the forma altogether
    bool isConversionSelected() const;

    static const int s_conversionNotSelected = -1;

Q_SIGNALS:
    // only signal called when any of the input/output strings has changed
    void changed();

private:
    ConversionManager* const m_conversionManager;
    AzureStorageManager* const m_storageManager;
    ConversionConfigModel* const m_conversionConfigModel;
    QStandardItemModel* const m_rootDirectoryModel;
    Configuration* const m_configuration;

    // directory used last in updateRootDirectoryModel as the model directory
    bool m_inhibitCurrentInputRootDirectory = false;
    QString m_lastModelDirectory;

    int m_conversionId = s_conversionNotSelected;


    const Conversion* getConversion() const;

    // load the conversion config file for the passed conversion
    void loadConfigFileForConversion(const Conversion* conversion);

    void updateRootDirectoryModel();
};
