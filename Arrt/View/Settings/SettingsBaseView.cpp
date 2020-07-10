#include <QHBoxLayout>
#include <QLineEdit>
#include <Utils/ScopedBlockers.h>
#include <View/Parameters/BoundWidget.h>
#include <View/Parameters/BoundWidgetFactory.h>
#include <View/Settings/SettingsBaseView.h>
#include <ViewModel/Parameters/ParameterModel.h>
#include <ViewModel/Settings/SettingsBaseModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/ReadOnlyText.h>

SettingsBaseView::SettingsBaseView(SettingsBaseModel* baseModel, QWidget* parent)
    : QWidget(parent)
    , m_baseModel(baseModel)
{
    m_topLayout = new QHBoxLayout();
    setLayout(m_topLayout);

    m_listLayout = new QVBoxLayout();
    m_statusLayout = new QHBoxLayout();

    m_statusBar = new QFrame();
    m_statusBar->setFrameShape(QFrame::VLine);

    setStatusBarColor(palette().midlight().color());

    m_topLayout->addWidget(m_statusBar, 0);
    m_topLayout->addLayout(m_listLayout);

    m_status = new ReadOnlyText();
    m_statusLayout->addWidget(m_status);

    m_statusLayout->setSpacing(3);

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

void SettingsBaseView::setStatusBarColor(const QColor& color)
{
    auto palette = m_statusBar->palette();
    palette.setColor(QPalette::WindowText, color);
    m_statusBar->setPalette(palette);
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
