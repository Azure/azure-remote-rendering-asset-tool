#pragma once
#include <QWidget>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>

class StatsPageModel;
class ParameterWidget;
class StatsGraph;
class QLabel;
class QVBoxLayout;

// widget used to show a group of parameters in a stat panel, with its own graph

class ParametersWidget : public QWidget
{
    Q_OBJECT
public:
    ParametersWidget(StatsPageModel* model, QString name, QWidget* parent = {});
    void addParameter(StatsPageModel::ValueType type);
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
    std::vector<StatsPageModel::ValueType> m_valueTypes;
    std::vector<QLabel*> m_labels;
    std::vector<ParameterWidget*> m_parameters;

    QVBoxLayout* m_parametersLayout;
    StatsGraph* m_graph;
    bool m_graphPerWindow = false;
    bool m_isSelected = false;

    void setAccessibleName(QString name);
};
