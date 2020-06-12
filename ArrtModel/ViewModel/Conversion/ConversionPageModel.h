#pragma once
#include <QObject>

class ConversionModel;
class CurrentConversionsModel;
class ConversionManager;
class AzureStorageManager;
class QItemSelectionModel;
class Configuration;
class NotificationButtonModel;
class NotificationButtonModelImplementation;

// Model for ConversionPageView, the page showing current/past conversions and selected conversion

class ConversionPageModel : public QObject
{
    Q_OBJECT

public:
    ConversionPageModel(ConversionManager* conversionManager, AzureStorageManager* storageManager, Configuration* configuration, QObject* parent);

    NotificationButtonModel* getNotificationButtonModel() const;

    void addNewConversion();

    bool canRemoveCurrentConversion() const;
    void removeCurrentConversion();

    CurrentConversionsModel* getCurrentConversionsModel() const;
    QItemSelectionModel* getSelectionModel() const;
    ConversionModel* getSelectedConversionModel() const;

    bool isEnabled() const;

Q_SIGNALS:
    void onEnabledChanged();
    void changed();

private:
    ConversionManager* const m_conversionManager;
    QItemSelectionModel* const m_selectionModel;
    CurrentConversionsModel* const m_currentConversionModels;
    ConversionModel* const m_conversionModel;
    NotificationButtonModelImplementation* const m_buttonModel;

    // number of succeeded conversions to notify when the panel is not shown
    int m_successCount = 0;
    // number of failed conversions to notify when the panel is not shown
    int m_failureCount = 0;

    // caches the old value of canRemoveCurrentConversion to know if it changed
    bool m_couldRemoveCurrentConversion = false;
    int getCurrentConversionId() const;

    // check if changed needs to be triggered and triggers it if it's the case
    void triggerChanged();

    void updateButton();
};
