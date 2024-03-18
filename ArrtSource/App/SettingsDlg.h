#pragma once

#include <QDialog>

#include "ui_SettingsDlg.h"

class StorageAccount;
class ArrAccount;

struct ConnectionProfile
{
    QString m_arrAccountId;
    QString m_arrAccountKey;
    QString m_arrAccountDomain;
    QString m_arrRegion;

    QString m_storageAccountName;
    QString m_storageAccountKey;
    QString m_storageEndpointUrl;
};

/// The modal dialog where users input their account credentials
class SettingsDlg : public QDialog, Ui_SettingsDlg
{
    Q_OBJECT
public:
    SettingsDlg(StorageAccount* storage, ArrAccount* arrClient, QWidget* parent = {});
    ~SettingsDlg();

private Q_SLOTS:
    void on_Buttons_rejected();
    void on_TestArr_clicked();
    void on_TestStorage_clicked();
    void on_AddProfile_clicked();
    void on_RemoveProfile_clicked();
    void on_ProfileDropDown_currentIndexChanged(int index);

private:
    bool ApplyArr();
    void ApplyStorage();
    void FillProfiles();
    void SetActiveProfile(const QString& name);
    void PullProfile();
    void PushProfile();
    void SaveProfiles();
    void LoadProfiles();

protected:
    virtual void closeEvent(QCloseEvent*) override;

    StorageAccount* m_storageAccount = nullptr;
    ArrAccount* m_arrClient = nullptr;

    QString m_activeProfile = "<default>";
    std::map<QString, ConnectionProfile> m_profiles;
};
