#pragma once
#include <QWidget>

class StatsPageModel;
class ParameterWidget;
class StatsGraph;
class QLabel;
class QVBoxLayout;

class ParametersWidget : public QWidget
{
    Q_OBJECT
public:
    ParametersWidget(StatsPageModel* model, QWidget* parent = {});
    void addParameter(int index);
    void updateUi();
    void setGraphPerWindow(bool perWindow);
    void setSelected(bool selected);

    static QWidget* createHeader(QWidget* parent);

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
