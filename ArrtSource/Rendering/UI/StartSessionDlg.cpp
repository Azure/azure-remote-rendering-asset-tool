#include "StartSessionDlg.h"
#include <AzureRemoteRendering.h>
#include <QMessageBox>
#include <QSettings>
#include <Rendering/ArrSession.h>
#include <Rendering/ArrSettings.h>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <Utils/TimeValidator.h>

using ArrVmSize = RR::RenderingSessionVmSize;

StartSessionDlg::StartSessionDlg(ArrSession* arrSession, ArrSettings* settings, QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    m_vmSize = (int)ArrVmSize::Standard;

    LoadState();

    m_arrSettings = settings;
    m_arrSession = arrSession;

    QObject::connect(m_arrSession, &ArrSession::SessionStatusChanged, this, [this]()
                     { UpdateUi(); });

    VmSize->addItem(tr("Standard"), QVariant((int)ArrVmSize::Standard));
    VmSize->addItem(tr("Premium"), QVariant((int)ArrVmSize::Premium));

    // VM Size
    {
        for (int i = 0; i < VmSize->count(); ++i)
        {
            if (VmSize->itemData(i).toInt() == m_vmSize)
            {
                VmSize->setCurrentIndex(i);
                break;
            }
        }
    }

    // lease time
    {
        MaxTime->setValidator(new TimeValidator(this));
        MaxTime->setText(TimeValidator::minutesToString(m_maxLeaseMinutes));
    }

    // extend time
    {
        AutoExtendBy->setVisible(false); // not exposed in UI at the moment
        ExtendNow->setVisible(false);
        ExtendLabel->setVisible(false);

        AutoExtendBy->blockSignals(true);
        AutoExtendBy->setValidator(new TimeValidator(this));
        AutoExtendBy->setText(TimeValidator::minutesToString(m_extendMinutes));
        AutoExtendBy->blockSignals(false);
    }

    // auto extend
    {
        AutoExtend->blockSignals(true);
        AutoExtend->setChecked(m_autoExtend);
        AutoExtend->blockSignals(false);
    }

    // Video settings
    {
        // it easily is possible to select invalid resolutions, then the render connection will fail
        // for the time being, selecting a custom resolution is simply disabled

        ResolutionX->setVisible(false);
        ResolutionY->setVisible(false);
        RefreshRate->setVisible(false);
        ResolutionXLabel->setVisible(false);
        ResolutionYLabel->setVisible(false);
        RefreshRateLabel->setVisible(false);

        ResolutionX->setMinimum(ArrSettings::s_videoWidthMin);
        ResolutionX->setMaximum(ArrSettings::s_videoWidthMax);
        ResolutionY->setMinimum(ArrSettings::s_videoHeightMin);
        ResolutionY->setMaximum(ArrSettings::s_videoHeightMax);
        RefreshRate->setMinimum(ArrSettings::s_videoRefreshRateMin);
        RefreshRate->setMaximum(ArrSettings::s_videoRefreshRateMax);

        ResolutionX->setValue(m_arrSettings->GetVideoWidth());
        ResolutionY->setValue(m_arrSettings->GetVideoHeight());
        RefreshRate->setValue(m_arrSettings->GetVideoRefreshRate());
    }

    UpdateUi();

    SessionID->selectAll();
}

StartSessionDlg::~StartSessionDlg() = default;

void StartSessionDlg::on_StartSession_clicked()
{
    StartSession->setEnabled(false);

    m_vmSize = VmSize->currentData().toInt();
    m_maxLeaseMinutes = TimeValidator::stringToMinutes(MaxTime->text());

    m_arrSettings->SetVideoWidth(ResolutionX->value());
    m_arrSettings->SetVideoHeight(ResolutionY->value());
    m_arrSettings->SetVideoRefreshRate(RefreshRate->value());
    m_arrSettings->SaveSettings();

    RR::RenderingSessionCreationOptions param;
    param.Size = (RR::RenderingSessionVmSize)m_vmSize;
    param.MaxLeaseInMinutes = m_maxLeaseMinutes;

    SetAutoExtension();

    if (SessionID->text().isEmpty())
    {
        m_arrSession->CreateSession(param);
    }
    else
    {
        m_arrSession->OpenSession(SessionID->text());
    }

    SaveState();

    accept();
}

void StartSessionDlg::on_StopSession_clicked()
{
    auto res = QMessageBox::question(this, "Stop Session?", "After disconnecting from the session, should it also be stopped?\n\nIf the session keeps running, you can reconnect to it quickly, using its session ID. However, if you don't intend to connect to it again, it should be stopped to not incur further costs.", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

    if (res == QMessageBox::Cancel)
        return;

    StopSession->setEnabled(false);

    if (res == QMessageBox::Yes)
    {
        m_arrSession->CloseSession(false);
    }
    else
    {
        m_arrSession->CloseSession(true);
    }

    // reset to original selected state
    MaxTime->setText(TimeValidator::minutesToString(m_maxLeaseMinutes));

    accept();
}

void StartSessionDlg::on_AutoExtend_stateChanged(int)
{
    if (m_arrSession->GetConnectionState().IsConnectionStoppable())
    {
        SetAutoExtension();
    }
}

void StartSessionDlg::on_AutoExtendBy_editingFinished()
{
    if (m_arrSession->GetConnectionState().IsConnectionStoppable())
    {
        SetAutoExtension();
    }
}

void StartSessionDlg::on_ExtendNow_clicked()
{
    const auto& status = m_arrSession->GetConnectionState();

    if (status.IsConnectionStoppable())
    {
        m_arrSession->ChangeSessionLeaseTime(status.GetLeaseTimeInMinutes() + m_extendMinutes);
    }
}

void StartSessionDlg::SaveState()
{
    QSettings s;
    s.beginGroup("SessionSettings");
    s.setValue("MaxLeaseMinutes", m_maxLeaseMinutes);
    s.setValue("AutoExtend", m_autoExtend);
    s.setValue("AutoExtendMinutes", m_extendMinutes);
    s.setValue("VmSize", m_vmSize);
    s.endGroup();
}

void StartSessionDlg::LoadState()
{
    QSettings s;
    s.beginGroup("SessionSettings");
    m_maxLeaseMinutes = s.value("MaxLeaseMinutes", m_maxLeaseMinutes).toInt();
    m_autoExtend = s.value("AutoExtend", m_autoExtend).toBool();
    m_extendMinutes = s.value("AutoExtendMinutes", m_extendMinutes).toInt();
    m_vmSize = s.value("VmSize", m_vmSize).toInt();
    s.endGroup();
}

void StartSessionDlg::UpdateUi()
{
    const auto& status = m_arrSession->GetConnectionState();
    const bool busy = status.IsConnectionActive();
    const bool stoppable = status.IsConnectionStoppable();

    StartSession->setEnabled(!busy);
    VmSize->setEnabled(!busy);
    MaxTime->setEnabled(!busy);
    StopSession->setEnabled(stoppable);
    ExtendNow->setEnabled(busy);
    ResolutionX->setEnabled(!busy);
    ResolutionY->setEnabled(!busy);
    RefreshRate->setEnabled(!busy);
    SessionID->setReadOnly(busy);

    if (busy)
    {
        if (SessionID->text() != m_arrSession->GetSessionID()) // accessibility fix to not lose text highlight when session update timer fires
        {
            SessionID->setPlaceholderText({});
            SessionID->setText(m_arrSession->GetSessionID());
        }

        MaxTime->setText(TimeValidator::minutesToString(status.GetLeaseTimeInMinutes()));
        RemainingTime->setText(TimeValidator::minutesToString(status.GetLeaseTimeInMinutes() - status.GetElapsedTimeInMinutes()));
    }
    else
    {
        SessionID->setText("");
        SessionID->setPlaceholderText("<no session running>");
        RemainingTime->setText("");
    }
}

void StartSessionDlg::SetAutoExtension()
{
    m_extendMinutes = TimeValidator::stringToMinutes(AutoExtendBy->text());

    if (m_extendMinutes < 1)
    {
        m_extendMinutes = 1;
    }

    m_autoExtend = AutoExtend->isChecked();

    m_arrSession->SetAutoExtensionMinutes(m_autoExtend ? m_extendMinutes : 0);

    SaveState();
}
