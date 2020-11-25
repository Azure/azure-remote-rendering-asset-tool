#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QStylePainter>
#include <QVBoxLayout>
#include <QValidator>
#include <View/ArrtStyle.h>
#include <View/Session/RunningSessionView.h>
#include <ViewModel/Session/RunningSessionModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FlowLayout.h>
#include <Widgets/FormControl.h>
#include <Widgets/HoursMinutesControl.h>
#include <Widgets/ReadOnlyText.h>

Q_DECLARE_METATYPE(SessionModel::Size);

RunningSessionView::RunningSessionView(RunningSessionModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    auto* panelLayout = new QVBoxLayout(this);

    {
        m_currentSessionId = new ReadOnlyText();
        auto* fc = new FormControl(tr("Session ID"), m_currentSessionId);
        fc->setToolTip(tr("Session ID"), tr("Session ID of the running session"));
        panelLayout->addWidget(fc);
    }

    {
        m_sizeInfo = new ReadOnlyText();
        auto* fc = new FormControl(tr("Size"), m_sizeInfo);
        fc->setToolTip(tr("VM Size"), tr("Size of the Virtual Machine used for this remote rendering session"));
        panelLayout->addWidget(fc);
    }

    {
        m_maxTime = new HoursMinutesControl();
        m_maxTime->setReadOnly(true);
        auto* fc = new FormControl(tr("Max Time (HH:MM)"), m_maxTime);
        fc->setToolTip(tr("Maximum lease time"), tr("Maximum lease time for this remote rendering session. Once the lease time expires, the session will be stopped"));
        panelLayout->addWidget(fc);
    }

    {
        m_remainingTime = new HoursMinutesControl();
        m_remainingTime->setEnabled(false);
        auto* fc = new FormControl(tr("Remaining time (HH:MM)"), m_remainingTime);
        fc->setToolTip(tr("Remaining lease time"), tr("Number of remaining minutes before the session is stopped"));
        panelLayout->addWidget(fc);
    }


    m_extendTime = new HoursMinutesControl();
    m_extendTime->setAccessibleName(tr("Extend Time"));
    connect(m_extendTime, &HoursMinutesControl::editingFinished, this, [this]() {
        m_model->setExtensionTime(m_extendTime->getMinutes());
    });

    m_extendButton = new FlatButton(tr("Extend"));
    m_extendButton->setToolTip(tr("Extend the session lease time"), tr("Add the specified amount of minutes to the session lease time"));
    m_extendButton->setIcon(ArrtStyle::s_extendtimeIcon, true);
    connect(m_extendButton, &FlatButton::clicked, this, [this]() {
        m_model->extend();
    });

    m_automaticExtend = new FlatButton(tr("Auto Extend"));
    m_automaticExtend->setToolTip(tr("Automatic extension of lease time"), tr("When active, the session will be automatically extended by the number of minutes specified, when it\'s close to the expiring time"));
    m_automaticExtend->setIcon(ArrtStyle::s_autoextendtimeIcon, true);
    m_automaticExtend->setCheckable(true);

    connect(m_automaticExtend, &FlatButton::toggled, this, [this](bool checked) {
        m_model->setAutomaticallyExtended(checked);
    });

    {
        auto* ext_l = new FlowLayout();
        ext_l->addWidget(m_extendTime);
        ext_l->addWidget(m_extendButton);
        ext_l->addWidget(m_automaticExtend);
        auto* fc = new FormControl(tr("Extend Time (HH:MM)"), ext_l);
        fc->setToolTip(tr("Extend lease time"), tr("Number of minutes to be added to the maximum lease time either automatically when \"Automatic Extension\" is active, or manually, by pressing the \"Extend\" button"));
        panelLayout->addWidget(fc);
    }

    {
        m_lastMessage = new QLineEdit();
        m_lastMessage->setReadOnly(true);
        auto* fc = new FormControl(tr("Message"), m_lastMessage);
        fc->setToolTip(tr("Session message"), tr("Message exposed by the remote rendering session"));
        panelLayout->addWidget(fc);
    }

    panelLayout->addStretch(1);

    m_arrInspectorButton = new FlatButton(tr("Inspect Session"));
    m_arrInspectorButton->setToolTip(tr("Inspect Running Session"), tr("Open the ArrInspector tool on a local browser, to visualize diagnostic data on this running session"));
    m_arrInspectorButton->setIcon(ArrtStyle::s_inspectorIcon, true);
    panelLayout->addWidget(m_arrInspectorButton);
    connect(m_arrInspectorButton, &FlatButton::clicked, this, [this]() {
        m_model->startInspector();
    });

    m_startStopButton = new FlatButton(tr("Stop Session"));
    m_startStopButton->setToolTip(tr("Stop session"), tr("Stop the currently running session. This brings you to the main panel to start a new session"));
    m_startStopButton->setIcon(ArrtStyle::s_stopIcon, true);
    connect(m_startStopButton, &FlatButton::clicked, this, [this]() { stop(); });

    panelLayout->addWidget(m_startStopButton);

    connect(m_model, &SessionModel::changed, this, [this]() { updateUi(); });
    updateUi();
}


void RunningSessionView::stop()
{
    if (m_model->isRunning())
    {
        m_model->stop();
    }
}

void RunningSessionView::updateUi()
{
    QString label;
    bool isRunning = m_model->isRunning();

    m_currentSessionId->setText(isRunning ? m_model->getSessionId() : QString());

    m_sizeInfo->setText(m_model->getSize() == SessionModel::Size::Standard ? tr("Standard") : tr("Premium"));

    m_maxTime->setMinutes(m_model->getLeaseTime().m_totalMinutes);
    m_maxTime->setEnabled(!isRunning);

    m_remainingTime->setMinutes(m_model->getRemainingTime().m_totalMinutes);

    m_extendTime->setMinutes(m_model->getExtensionTime().m_totalMinutes);

    m_automaticExtend->setChecked(m_model->isAutomaticallyExtended());
    m_extendButton->setEnabled(isRunning);

    m_lastMessage->setText(m_model->getLastMessage());

    m_arrInspectorButton->setEnabled(m_model->canInspectSession());

    m_startStopButton->setEnabled(isRunning);
}
