#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <Utils/ScopedBlockers.h>
#include <View/ArrtStyle.h>
#include <View/Parameters/BoundWidget.h>
#include <View/Parameters/BoundWidgetFactory.h>
#include <View/Settings/SettingsBaseView.h>
#include <ViewModel/Parameters/ParameterModel.h>
#include <ViewModel/Settings/SettingsBaseModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/ReadOnlyText.h>

// widget used to draw a coloured vertical line

class StatusIndicator : public QWidget
{
public:
    StatusIndicator(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumWidth(2);
    }

    void setColor(QColor color)
    {
        m_color = color;
        update();
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setPen(QPen(m_color, 2));
        QRect r = rect();
        p.drawLine(QPoint(1, r.top()), QPoint(1, r.bottom()));
    }

private:
    QColor m_color = Qt::white;
};

SettingsBaseView::SettingsBaseView(SettingsBaseModel* baseModel, QWidget* parent)
    : QWidget(parent)
    , m_baseModel(baseModel)
{
    m_topLayout = new QHBoxLayout();
    setLayout(m_topLayout);

    m_listLayout = new QVBoxLayout();
    m_statusLayout = new QHBoxLayout();

    m_statusBar = new StatusIndicator();

    m_topLayout->addWidget(m_statusBar, 0);
    m_topLayout->addLayout(m_listLayout);

    m_status = new ReadOnlyText();
    m_status->setAccessibleName(tr("Status"));
    m_statusLayout->addWidget(m_status);

    m_statusLayout->setSpacing(3);
    setStatus(NEUTRAL, "");

    {
        auto* fc = new FormControl(tr("Status"), m_statusLayout);
        fc->setToolTip(tr("Connection status"), tr("Indicates the current status of the connection"));
        m_listLayout->addWidget(fc);
    }

    for (const auto& controlModel : m_baseModel->getControls())
    {
        FormControl* w = new FormControl(this);
        m_listLayout->addWidget(w);
        w->setHeader(controlModel->getName());
        w->setWidget(BoundWidgetFactory::createBoundWidget(controlModel, w));
        m_widgets.push_back(w);
    }

    QObject::connect(m_baseModel, &SettingsBaseModel::updateUi, this, [this]() {
        updateUi();
    });
    updateUi();
}

void SettingsBaseView::setStatus(Status status, QString description)
{
    switch (status)
    {
        case NEUTRAL:
            m_statusBar->setColor(palette().midlight().color());
            break;
        case OK:
            m_statusBar->setColor(ArrtStyle::s_connectedColor);
            break;
        case INPROGRESS:
            m_statusBar->setColor(ArrtStyle::s_connectingColor);
            break;
        case ERROR:
            m_statusBar->setColor(ArrtStyle::s_disconnectedColor);
            break;
    }
    m_status->setText(description);
}

void SettingsBaseView::updateUi()
{
    const bool canEdit = m_baseModel->isEnabled();
    for (FormControl* w : m_widgets)
    {
        ScopedBlockSignals scopedBlockSignals(w->getWidget());
        if (auto* bw = qobject_cast<BoundWidget*>(w->getWidget()))
        {
            bw->updateFromModel();
        }
        w->setEnabled(canEdit);
    }
}
