#pragma once
#include <QWidget>

class ReadOnlyText;

// Row in a ParametersWidget, with stats for a single parameter

class ParameterWidget : public QWidget
{
public:
    ParameterWidget(QString name, QString unit, QColor color, QWidget* parent = {});
    void setLegendVisibility(bool visible);
    void setValues(float value, float minValue, float maxValue, float averageValue);

    static QWidget* createHeader(QWidget* parent);

private:
    QWidget* m_legend;
    ReadOnlyText* m_valueLabel;
    ReadOnlyText* m_minLabel;
    ReadOnlyText* m_maxLabel;
    ReadOnlyText* m_averageLabel;
    QString m_unit;
};
