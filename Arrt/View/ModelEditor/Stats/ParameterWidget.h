#pragma once
#include <QWidget>

class QLabel;

class ParameterWidget : public QWidget
{
public:
    ParameterWidget(QString name, QString unit, QColor color, QWidget* parent = {});
    void setLegendVisibility(bool visible);
    void setValues(float value, float minValue, float maxValue, float averageValue);

    static QWidget* createHeader(QWidget* parent);

private:
    QWidget* m_legend;
    QLabel* m_valueLabel;
    QLabel* m_minLabel;
    QLabel* m_maxLabel;
    QLabel* m_averageLabel;
    QString m_unit;
};
