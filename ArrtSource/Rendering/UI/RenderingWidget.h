#pragma once

#include "ui_RenderingWidget.h"

class RenderingWidget : public QWidget, public Ui_RenderingWidget
{
    Q_OBJECT

public:
    RenderingWidget(QWidget* parent = {})
        : QWidget(parent)
    {
        setupUi(this);
    }
};
