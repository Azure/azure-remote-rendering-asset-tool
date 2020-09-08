#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <View/ArrtStyle.h>
#include <View/ModelEditor/Stats/ParameterWidget.h>
#include <View/ModelEditor/Stats/ParametersWidget.h>
#include <View/ModelEditor/Stats/StatsPageView.h>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/VerticalScrollArea.h>

StatsPageView::StatsPageView(StatsPageModel* statsPageModel, QWidget* parent)
    : QWidget(parent)
    , m_model(statsPageModel)
{
    setContentsMargins(QMargins());

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());

    auto* buttonsLayout = new QHBoxLayout();

    auto* showButton = new FlatButton({}, this);
    buttonsLayout->addWidget(showButton);

    auto* timeAxis = new QComboBox(this);

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(timeAxis);

    mainLayout->addLayout(buttonsLayout);

    auto* statsPanel = new VerticalScrollArea(this);

    mainLayout->addWidget(ParameterWidget::createHeader(this));

    {
        auto* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(separator);
    }

    {
        auto* l = statsPanel->getContentLayout();

        ParametersWidget* widget = nullptr;
        for (int i = 0; i < m_model->getParameterCount(); ++i)
        {
            if (!widget)
            {
                const int idx = m_graphs.size();
                widget = new ParametersWidget(m_model, statsPanel);
                l->addWidget(widget);
                m_graphs.push_back(widget);

                connect(widget, &ParametersWidget::onFocus, [this, idx](bool focused) {
                    if (focused)
                    {
                        setSelectedGraph(idx);
                    }
                });
            }

            widget->addParameter(i);
            if (m_model->getPlotInfo(i).m_endGroup)
            {
                widget = nullptr;
            }
        }

        connect(m_model, &StatsPageModel::valuesChanged, this, [this]() { updateUi(); });

        updateUi();
    }

    mainLayout->addWidget(statsPanel);


    mainLayout->addWidget(statsPanel);
    auto toggleCollection = [this, showButton, statsPanel](bool checked) {
        if (checked)
        {
            m_model->startCollecting();
            showButton->setIcon(ArrtStyle::s_stopIcon, true);
            showButton->setText(tr("Stop statistics collection"));
        }
        else
        {
            m_model->stopCollecting();
            showButton->setIcon(ArrtStyle::s_startIcon, true);
            showButton->setText(tr("Start statistics collection"));
        }
    };
    showButton->setCheckable(true);
    showButton->setChecked(false);
    connect(showButton, &FlatButton::toggled, this, toggleCollection);
    toggleCollection(false);

    auto togglePerSecond = [this, statsPanel](bool checked) {
        for (auto&& graph : m_graphs)
        {
            graph->setGraphPerWindow(checked);
        }
        updateUi();
    };
    timeAxis->addItem(tr("Show stats per frame"));
    timeAxis->addItem(tr("Show stats per second"));
    connect(timeAxis, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [togglePerSecond](int index) {
        togglePerSecond(index == 1);
    });
    timeAxis->setCurrentIndex(0);

    setSelectedGraph(0);
}

StatsPageView::~StatsPageView()
{
}

void StatsPageView::updateUi()
{
    for (auto&& graph : m_graphs)
    {
        graph->updateUi();
    }
}

void StatsPageView::setSelectedGraph(int idx)
{
    if (m_selectedGraph != idx)
    {
        if (m_selectedGraph >= 0)
        {
            m_graphs[m_selectedGraph]->setSelected(false);
        }

        m_selectedGraph = idx;

        if (m_selectedGraph >= 0)
        {
            m_graphs[m_selectedGraph]->setSelected(true);
        }
    }
}
