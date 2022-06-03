#include "CameraDlg.h"
#include <QMessageBox>
#include <Rendering/ArrSettings.h>

CameraDlg::CameraDlg(ArrSettings* options, QWidget* parent)
    : QDialog(parent)
    , m_options(options)
{
    setupUi(this);

    FieldOfViewSlider->blockSignals(true);
    NearPlaneSlider->blockSignals(true);
    FarPlaneSlider->blockSignals(true);
    CamMoveSpeedSlider->blockSignals(true);

    // in degrees
    FieldOfViewSlider->setMinimum(30);
    FieldOfViewSlider->setMaximum(160);

    // in cm
    NearPlaneSlider->setMinimum(1);
    NearPlaneSlider->setMaximum(100);

    // in cm
    FarPlaneSlider->setMinimum(500);
    FarPlaneSlider->setMaximum(50000);

    CamMoveSpeedSlider->setMinimum(0);
    CamMoveSpeedSlider->setMaximum(12);

    updateUi();

    FieldOfViewSlider->blockSignals(false);
    NearPlaneSlider->blockSignals(false);
    FarPlaneSlider->blockSignals(false);
    CamMoveSpeedSlider->blockSignals(false);
}

CameraDlg::~CameraDlg() = default;

void CameraDlg::updateUi()
{
    FieldOfViewSlider->setValue(m_options->GetFovAngle());
    NearPlaneSlider->setValue(m_options->GetNearPlaneCM());
    FarPlaneSlider->setValue(m_options->GetFarPlaneCM());
    CamMoveSpeedSlider->setValue(m_options->GetCameraSpeedInPow2());
    PointSizeSlider->setValue(m_options->GetPointSize());
    updateLabels();
}

void CameraDlg::updateLabels()
{
    FovLabel->setText(QString("%1 degree").arg(m_options->GetFovAngle()));
    NearPlaneLabel->setText(QString("%1 meters").arg(m_options->GetNearPlaneCM() / 100.0f));
    FarPlaneLabel->setText(QString("%1 meters").arg(m_options->GetFarPlaneCM() / 100.0f));
    CamSpeedLabel->setText(QString("%1 m/sec").arg(m_options->GetCameraSpeedMetersPerSecond()));
    PointSizeLabel->setText(QString("%1").arg(m_options->GetPointSize() / 10.0, 0, 'f', 1));
}

void CameraDlg::closeEvent(QCloseEvent*)
{
    m_options->SaveSettings();
}

void CameraDlg::on_Buttons_rejected()
{
    closeEvent(nullptr);
    accept();
}

void CameraDlg::on_FieldOfViewSlider_valueChanged(int value)
{
    m_options->SetFovAngle(value);
    updateLabels();
}

void CameraDlg::on_NearPlaneSlider_valueChanged(int value)
{
    m_options->SetNearPlaneCM(value);
    updateLabels();
}

void CameraDlg::on_FarPlaneSlider_valueChanged(int value)
{
    m_options->SetFarPlaneCM(value);
    updateLabels();
}

void CameraDlg::on_CamMoveSpeedSlider_valueChanged(int value)
{
    m_options->SetCameraSpeedInPow2(value);
    updateLabels();
}

void CameraDlg::on_PointSizeSlider_valueChanged(int value)
{
    m_options->SetPointSize(value);
    updateLabels();
}
