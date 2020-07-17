#pragma once
#include <QFrame>
#include <QLineEdit>

class RunningSessionModel;
class HoursMinutesControl;
class FlatButton;
class SessionInfoButton;
class ReadOnlyText;

// panel used to visualize the status of the session and extend lease time or stop the session.

class RunningSessionView : public QWidget
{
public:
    RunningSessionView(RunningSessionModel* model, QWidget* parent = {});

private:
    RunningSessionModel* const m_model;
    FlatButton* m_startStopButton = {};
    ReadOnlyText* m_sizeInfo = {};
    ReadOnlyText* m_currentSessionId = {};
    HoursMinutesControl* m_maxTime = {};
    HoursMinutesControl* m_remainingTime = {};
    HoursMinutesControl* m_extendTime = {};
    FlatButton* m_automaticExtend = {};
    FlatButton* m_extendButton = {};
    QLineEdit* m_lastMessage = {};
    FlatButton* m_arrInspectorButton = {};

    void updateUi();
    void stop();
};
