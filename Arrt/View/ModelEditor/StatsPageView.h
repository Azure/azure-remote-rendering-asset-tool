#pragma once
#include <qwidget.h>

class StatsPageModel;
class QLabel;


class SimpleGraph : public QWidget
{
public:
    SimpleGraph(QWidget* parent = {});

    virtual void paintEvent(QPaintEvent* event) override;
    std::vector<QPointF>& accessPlotData();

private:
    std::vector<QPointF> m_data;
};

// panel with rendering statistics

class StatsPageView : public QWidget
{
public:
    StatsPageView(StatsPageModel* statsPageModel);
    ~StatsPageView();

private:
    StatsPageModel* const m_model;
    QList<QLabel*> m_values;
    QList<SimpleGraph*> m_graphs;
    QStringList m_units;

    void updateUi();
};
