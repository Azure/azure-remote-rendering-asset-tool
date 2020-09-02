#pragma once
#include <QWidget>
#include <ViewModel/ModelEditor/StatsPageModel.h>

class StatsPageModel;
class QLabel;
class QVBoxLayout;


class SimpleGraph : public QWidget
{
public:
    SimpleGraph(QWidget* parent = {});

    virtual void paintEvent(QPaintEvent* event) override;
    int getPlotCount() const;

    int addPlot(StatsPageModel::PlotInfo info);
    std::vector<QPointF>& accessPlotData(int index);

    void setMinMax(qreal minimum, qreal maximum);

private:
    std::vector<std::vector<QPointF>> m_data;
    std::vector<StatsPageModel::PlotInfo> m_infos;
    qreal m_minimum;
    qreal m_maximum;
};


class ParametersWidget : public QWidget
{
    Q_OBJECT
public:
    ParametersWidget(StatsPageModel* model, QWidget* parent = {});
    void addParameter(int index);
    void updateUi();
    void setGraphPerWindow(bool perWindow);
    void setSelected(bool selected);

Q_SIGNALS:
    void onFocus(bool focused);

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    StatsPageModel* const m_model;
    std::vector<int> m_indices;
    std::vector<QLabel*> m_labels;
    std::vector<QWidget*> m_legendColors;
    std::vector<QLabel*> m_values;
    QVBoxLayout* m_parametersLayout;
    SimpleGraph* m_graph;
    bool m_graphPerWindow = false;
    bool m_isSelected = false;
};

// panel with rendering statistics

class StatsPageView : public QWidget
{
public:
    StatsPageView(StatsPageModel* statsPageModel);
    ~StatsPageView();

private:
    struct GraphWithInfo
    {
        std::vector<int> m_parametersIndices;
        SimpleGraph* m_graph;
    };

    StatsPageModel* const m_model;
    QList<ParametersWidget*> m_graphs;
    int m_selectedGraph = -1;

    void updateUi();
    void setSelectedGraph(int idx);
};
