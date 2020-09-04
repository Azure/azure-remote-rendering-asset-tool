#pragma once
#include <QWidget>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>

class StatsPageModel;
class QLabel;
class QVBoxLayout;
class StatsGraph;

class ParameterWidget : public QWidget
{
public:
    ParameterWidget(QString name, QString unit, QColor color, QWidget* parent = {});
    void setLegendVisibility(bool visible);
    void setValues(float value, float minValue, float maxValue, float averageValue);

private:
    QWidget* m_legend;
    QLabel* m_valueLabel;
    QLabel* m_minLabel;
    QLabel* m_maxLabel;
    QLabel* m_averageLabel;
    QString m_unit;
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
    virtual void paintEvent(QPaintEvent* event) override;

private:
    StatsPageModel* const m_model;
    std::vector<int> m_indices;
    std::vector<QLabel*> m_labels;
    std::vector<ParameterWidget*> m_parameters;

    QVBoxLayout* m_parametersLayout;
	StatsGraph* m_graph;
    bool m_graphPerWindow = false;
    bool m_isSelected = false;
};

// panel with rendering statistics

class StatsPageView : public QWidget
{
public:
    StatsPageView(StatsPageModel* statsPageModel, QWidget* parent = {});
    ~StatsPageView();

private:
    struct GraphWithInfo
    {
        std::vector<int> m_parametersIndices;
		StatsGraph* m_graph;
    };

    StatsPageModel* const m_model;
    QList<ParametersWidget*> m_graphs;
    int m_selectedGraph = -1;

    void updateUi();
    void setSelectedGraph(int idx);
};
