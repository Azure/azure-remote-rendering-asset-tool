#pragma once

#include <QDialog>

#include "ui_CameraDlg.h"

class ArrSettings;

class CameraDlg : public QDialog, Ui_CameraDlg
{
    Q_OBJECT
public:
    CameraDlg(ArrSettings* options, QWidget* parent = {});
    ~CameraDlg();

private Q_SLOTS:
    void on_Buttons_rejected();
    void on_FieldOfViewSlider_valueChanged(int value);
    void on_NearPlaneSlider_valueChanged(int value);
    void on_FarPlaneSlider_valueChanged(int value);
    void on_CamMoveSpeedSlider_valueChanged(int value);

private:
    ArrSettings* m_options = nullptr;

    void updateUi();
    void updateLabels();

protected:
    virtual void closeEvent(QCloseEvent*) override;
};
