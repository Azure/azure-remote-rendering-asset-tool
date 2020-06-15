#pragma once
#include <QWidget>

class ParameterModel;

namespace BoundWidgetFactory
{
    // factory method which creates the right bound widget for a ParameterModel

    QWidget* createBoundWidget(ParameterModel* model, QWidget* parent);
} // namespace BoundWidgetFactory
