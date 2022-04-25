#pragma once

#include "ui_LogWidget.h"

class LogWidget : public QWidget, public Ui_LogWidget
{
    Q_OBJECT

public:
    LogWidget(QWidget* parent = {})
        : QWidget(parent)
    {
        setupUi(this);
    }
};
