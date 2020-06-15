#pragma once
#include <QComboBox>
#include <QWidget>

class SessionCreationModel;
class HoursMinutesControl;
class FlatButton;

// panel used to create the session. It is used to specify the parameters of a new session and start it

class SessionCreationView : public QWidget
{
public:
    SessionCreationView(SessionCreationModel* model, QWidget* parent = {});

private:
    SessionCreationModel* const m_model;
    FlatButton* m_startButton;
    QComboBox* m_sizeCombo;
    HoursMinutesControl* m_maxTime;
    HoursMinutesControl* m_extendTime;
    FlatButton* m_automaticExtend;

    void updateUi();
};
