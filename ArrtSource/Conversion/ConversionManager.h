#pragma once

#include <Conversion/Conversion.h>
#include <QObject>
#include <QTimer>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <deque>

class StorageAccount;
class ArrAccount;

class ConversionManager : public QObject
{
    Q_OBJECT

public:
    ConversionManager(StorageAccount* storageAccount, ArrAccount* arrClient);
    ~ConversionManager();

    const std::deque<Conversion>& GetConversions() const { return m_conversions; }
    const Conversion& GetSelectedConversion() const { return m_conversions[m_selectedConversion]; }

    uint32_t GetNumActiveConversions() const;

    void SetSelectedConversion(int selected);
    bool IsEditableSelected() const;
    bool StartConversion();

    void SetConversionName(const QString& name);
    void SetConversionSourceAsset(const QString& container, const QString& path);
    void SetConversionOutputFolder(const QString& container, const QString& path);
    void SetConversionAdvanced(bool advanced);
    void SetConversionAdvancedOptions(const ConversionOptions& options);

Q_SIGNALS:
    void SelectedChanged();
    void ListChanged();

private Q_SLOTS:
    void onCheckConversions();

private:
    bool StartConversionInternal();
    void SetConversionStatus(int conversionIdx, RR::Status status, RR::ApiHandle<RR::AssetConversionStatusResult> result);

    StorageAccount* m_storageAccount = nullptr;
    ArrAccount* m_arrClient = nullptr;
    QTimer m_checkConversionStateTimer;
    QTimer m_updateConversionListTimer;

    int m_selectedConversion = 0;
    std::deque<Conversion> m_conversions;
};