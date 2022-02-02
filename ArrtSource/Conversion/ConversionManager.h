#pragma once

#include <Conversion/Conversion.h>
#include <QObject>
#include <QTimer>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <deque>

class StorageAccount;
class ArrAccount;

/// Manages interactions with the model conversion service
class ConversionManager : public QObject
{
    Q_OBJECT

public:
    ConversionManager(StorageAccount* storageAccount, ArrAccount* arrClient);
    ~ConversionManager();

    /// Returns all recent conversions, both running and finished ones.
    const std::deque<Conversion>& GetConversions() const { return m_conversions; }

    /// Returns the conversion that is currently selected by the user.
    const Conversion& GetSelectedConversion() const { return m_conversions[m_selectedConversion]; }

    /// How many conversions are currently running (ie not finished)
    uint32_t GetNumActiveConversions() const;

    /// Changes the currently selected conversion.
    void SetSelectedConversion(int selected);

    /// Returns the index of the selected conversion.
    int GetSelectedConversionIndex() const { return m_selectedConversion; }

    /// Whether the selected conversion is editable, e.g. a not yet started one.
    bool IsEditableSelected() const;

    /// Starts the next conversion with the currently set options.
    bool StartConversion();

    void SetConversionName(const QString& name);
    void SetConversionSourceAsset(const QString& container, const QString& path);
    void SetConversionInputFolder(const QString& path);
    void SetConversionOutputFolder(const QString& container, const QString& path);
    void SetConversionAdvanced(bool advanced);
    void SetConversionAdvancedOptions(const ConversionOptions& options);

Q_SIGNALS:
    void SelectedChanged();
    void ListChanged();
    void ConversionFailed();
    void ConversionSucceeded();

private Q_SLOTS:
    void OnCheckConversions();

private:
    bool StartConversionInternal();
    void SetConversionStatus(int conversionIdx, RR::Status status, RR::ApiHandle<RR::ConversionPropertiesResult> result);
    void GetCurrentConversionsResult(RR::Status status, RR::ApiHandle<RR::ConversionPropertiesArrayResult> result);

    StorageAccount* m_storageAccount = nullptr;
    ArrAccount* m_arrClient = nullptr;
    QTimer m_checkConversionStateTimer;
    QTimer m_updateConversionListTimer;

    int m_selectedConversion = 0;
    std::deque<Conversion> m_conversions;
};