#pragma once

#include <QDialog>

#include "ui_StartSessionDlg.h"

class ArrSession;
class ArrSettings;

/// The modal dialog that lets you start/stop and extend rendering sessions.
class StartSessionDlg : public QDialog, Ui_StartSessionDlg
{
    Q_OBJECT
public:
    StartSessionDlg(ArrSession* arrSession, ArrSettings* settings, QWidget* parent = {});
    ~StartSessionDlg();

private Q_SLOTS:
    void on_StartSession_clicked();
    void on_StopSession_clicked();
    void on_AutoExtend_stateChanged(int);
    void on_AutoExtendBy_editingFinished();
    void on_ExtendNow_clicked();

private:
    void UpdateUi();
    void SetAutoExtension();
    void SaveState();
    void LoadState();

protected:
    ArrSession* m_arrSession = nullptr;
    ArrSettings* m_arrSettings = nullptr;

    int m_maxLeaseMinutes = 10;
    bool m_autoExtend = true;
    int m_extendMinutes = 5;
    int m_vmSize;
};
