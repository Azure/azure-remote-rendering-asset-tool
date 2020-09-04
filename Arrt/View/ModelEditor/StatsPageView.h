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
    void setPlotData(int index, std::vector<QPointF> plotData);

    void setMinMax(qreal minimum, qreal maximum);

    void setXLabel(QString text);
    void setXZoom(float zoom);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

    void setHighlightX(std::optional<float> x);
    QRect getGraphRect() const;

    void updateTransformAndGrid();

private:
    std::vector<std::vector<QPointF>> m_data;
    std::vector<StatsPageModel::PlotInfo> m_infos;
    qreal m_minimum;
    qreal m_maximum;
    QString m_xLabelText;
    QTransform m_currentTransform;
    QTransform m_currentTransformInverse;
    float m_xStep;
    float m_yStep;
    bool m_transformAndGridComputed = false;

    std::optional<float> m_highlightedXInPixels = {};
    std::optional<float> m_highlightedX = {};
    struct HighlightPoint
    {
        QColor m_color;
        QString m_text;
        QPointF m_pt;
    };
    std::vector<HighlightPoint> m_highlightedPoints;
    float m_xZoom = 2.0;
};

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
