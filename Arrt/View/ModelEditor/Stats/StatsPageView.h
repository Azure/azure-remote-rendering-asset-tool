#pragma once
#include <QWidget>

class StatsPageModel;
class ParametersWidget;

// panel with rendering statistics

class StatsPageView : public QWidget
{
public:
    StatsPageView(StatsPageModel* statsPageModel, QWidget* parent = {});
    ~StatsPageView();

private:
    StatsPageModel* const m_model;
    QList<ParametersWidget*> m_graphs;
    int m_selectedGraph = -1;

    void updateUi();
    void setSelectedGraph(int idx);
};
