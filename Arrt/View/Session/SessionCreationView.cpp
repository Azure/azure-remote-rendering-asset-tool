#include <QAbstractItemView>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <View/Session/SessionCreationView.h>
#include <ViewModel/Session/SessionCreationModel.h>
#include <ViewModel/Session/SessionModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/HoursMinutesControl.h>

Q_DECLARE_METATYPE(SessionModel::Size);

SessionCreationView::SessionCreationView(SessionCreationModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    auto* l = new QVBoxLayout(this);

    l->addWidget(ArrtStyle::createHeaderLabel(tr("Start new session"), tr("Start a new Azure Remote Rendering session to load and render a model")));

    {
        m_sizeCombo = new QComboBox();
        m_sizeCombo->addItem(tr("Standard"), QVariant::fromValue(SessionModel::Size::Standard));
        m_sizeCombo->addItem(tr("Premium"), QVariant::fromValue(SessionModel::Size::Premium));
        QObject::connect(m_sizeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
            m_model->setSize(m_sizeCombo->itemData(index).value<SessionModel::Size>());
        });
        m_sizeCombo->view()->setAccessibleName(tr("Size of VM"));

        auto* fc = new FormControl(tr("Size"), m_sizeCombo);
        fc->setToolTip(tr("VM Size"), tr("Size of the Virtual Machine used for this remote rendering session. It will affect the performance for bigger models and cannot be changed while the session is running"));
        l->addWidget(fc);
    }

    {
        m_maxTime = new HoursMinutesControl();
        QObject::connect(m_maxTime, &HoursMinutesControl::editingFinished, this, [this]() {
            m_model->setLeaseTime(m_maxTime->getMinutes());
        });
        auto* fc = new FormControl(tr("Max Time (HH:MM)"), m_maxTime);
        fc->setToolTip(tr("Maximum lease time"), tr("Initial maximum lease time for this remote rendering session. Once the lease time expires, the session will be stopped. It can be extended while running."));
        l->addWidget(fc);
    }

    {
        auto* ext_l = new QHBoxLayout();
        m_automaticExtend = new FlatButton(tr("Auto Extend"));
        m_automaticExtend->setToolTip(tr("Auto Extend"), tr("Toggle option to extend automatically the lease of the remote rendering session"));
        m_automaticExtend->setIcon(ArrtStyle::s_autoextendtimeIcon, true);
        m_automaticExtend->setCheckable(true);

        QObject::connect(m_automaticExtend, &FlatButton::toggled, this, [this](bool checked) {
            m_model->setAutomaticallyExtended(checked);
            updateUi();
        });
        ext_l->addWidget(m_automaticExtend);

        m_extendTime = new HoursMinutesControl();
        m_extendTime->setAccessibleName(tr("Extend Time"));
        QObject::connect(m_extendTime, &HoursMinutesControl::editingFinished, this, [this]() {
            m_model->setExtensionTime(m_extendTime->getMinutes());
        });
        ext_l->addWidget(m_extendTime);

        auto* fc = new FormControl(tr("Automatic Extension (HH:MM)"), ext_l);
        fc->setToolTip(tr("Automatic extension of lease time"), tr("When active, the session will be automatically extended by the number of minutes specified, when it\'s close to the expiring time"));
        l->addWidget(fc);
    }

    l->addStretch(1);

    m_startButton = new FlatButton(tr("Start session"));
    m_startButton->setToolTip(tr("Start session"), tr("Start the remote rendering session and automatically connects to it once it\'s ready"));
    m_startButton->setIcon(ArrtStyle::s_startIcon, true);

    QObject::connect(m_startButton, &FlatButton::clicked, this, [this]() {
        if (!m_model->isRunning())
        {
            m_model->start();
        }
        else
        {
            m_model->stop();
        } });

    l->addWidget(m_startButton);

    auto onEnabledChanged = [this]() {
        setEnabled(m_model->isEnabled());
    };
    onEnabledChanged();
    QObject::connect(m_model, &SessionCreationModel::onEnabledChanged, this, onEnabledChanged);

    QObject::connect(m_model, &SessionModel::changed, this, [this]() { updateUi(); });
    updateUi();
}

void SessionCreationView::updateUi()
{
    QString label;
    bool isRunning = m_model->isRunning();

    m_sizeCombo->setCurrentIndex(m_model->getSize() == SessionModel::Size::Standard ? 0 : 1);
    m_sizeCombo->setEnabled(!isRunning);

    m_maxTime->setMinutes(m_model->getLeaseTime().m_totalMinutes);
    m_maxTime->setEnabled(!isRunning);

    m_extendTime->setEnabled(m_model->isAutomaticallyExtended());
    m_extendTime->setMinutes(m_model->getExtensionTime().m_totalMinutes);
    m_automaticExtend->setChecked(m_model->isAutomaticallyExtended());

    m_startButton->setEnabled(!isRunning);
}
