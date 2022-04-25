#pragma once

#include "ui_ConversionWidget.h"

class ConversionWidget : public QWidget, public Ui_ConversionWidget
{
    Q_OBJECT

public:
    ConversionWidget(QWidget* parent = {})
        : QWidget(parent)
    {
        setupUi(this);
    }
};
