#pragma once

#include <QDialog>

#include "ui_SettingsDlg.h"

class StorageAccount;
class ArrAccount;

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

private:
    void ApplyArr();
    void ApplyStorage();

protected:
    virtual void closeEvent(QCloseEvent*) override;

    StorageAccount* m_storageAccount = nullptr;
    ArrAccount* m_arrClient = nullptr;
};
