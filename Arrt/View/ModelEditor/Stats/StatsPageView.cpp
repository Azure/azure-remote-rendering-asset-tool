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
    mainLayout->setSpacing(0);

    auto* buttonsLayout = new QHBoxLayout();

    auto* startStopButton = new FlatButton({}, this);
    buttonsLayout->addWidget(startStopButton);

    auto* autoCollectButton = new FlatButton({}, this);
    buttonsLayout->addWidget(autoCollectButton);

    auto* timeAxis = new QComboBox(this);
    timeAxis->setAccessibleName(tr("Stats aggregation type"));

    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(timeAxis);

    mainLayout->addLayout(buttonsLayout);

    auto* statsPanel = new VerticalScrollArea(this);

    auto* header = ParameterWidget::createHeader(this);
    mainLayout->addWidget(header);

    {
        auto* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(separator);
    }

    {
        auto* l = statsPanel->getContentLayout();

        for (int i = 0; i < m_model->getPlotGroupCount(); ++i)
        {
            const int idx = m_graphs.size();

            auto& groupInfo = m_model->getPlotGroup(i);
            ParametersWidget* widget = new ParametersWidget(m_model, groupInfo.m_name, statsPanel);
            l->addWidget(widget);
            m_graphs.push_back(widget);

            connect(widget, &ParametersWidget::onFocus, [this, idx](bool focused) {
                if (focused)
                {
                    setSelectedGraph(idx);
                }
            });

            for (auto& valueType : groupInfo.m_plots)
            {
                widget->addParameter(valueType);
            }
        }

        connect(m_model, &StatsPageModel::valuesChanged, this, [this]() { updateUi(); });

        updateUi();
    }

    mainLayout->addWidget(statsPanel);

    {
        auto toggleCollection = [this, startStopButton, statsPanel](bool checked) {
            if (checked)
            {
                m_model->startCollecting();
            }
            else
            {
                m_model->stopCollecting();
            }
        };
        startStopButton->setCheckable(true);
        connect(startStopButton, &FlatButton::toggled, this, toggleCollection);

        auto updateCollectingState = [this, startStopButton] {
            if (m_model->isCollecting())
            {
                startStopButton->setChecked(true);
                startStopButton->setIcon(ArrtStyle::s_stopIcon, true);
                startStopButton->setText(tr("Stop statistics collection"));
            }
            else
            {
                startStopButton->setChecked(false);
                startStopButton->setIcon(ArrtStyle::s_startIcon, true);
                startStopButton->setText(tr("Start statistics collection"));
            }
        };
        connect(m_model, &StatsPageModel::collectingStateChanged, this, updateCollectingState);
        updateCollectingState();
    }

    {
        autoCollectButton->setCheckable(true);
        connect(autoCollectButton, &FlatButton::toggled, this, [autoCollectButton, this](bool checked) {
            if (checked)
            {
                m_model->startAutoCollect();
            }
            else
            {
                m_model->stopAutoCollect();
            }
        });

        auto updateAutoCollectState = [autoCollectButton, this] {
            autoCollectButton->setChecked(m_model->isAutoCollecting());
            autoCollectButton->setText(m_model->getAutoCollectText());
            autoCollectButton->setIcon(m_model->isAutoCollecting() ? ArrtStyle::s_stopIcon : ArrtStyle::s_startIcon, true);
        };

        connect(m_model, &StatsPageModel::autoCollectStateChanged, this, updateAutoCollectState);
        updateAutoCollectState();
    }

    {
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
    }
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
