#pragma once
#include <QAccessible>
#include <QAccessibleWidget>

class QComboBox;

namespace ArrtAccesibility
{
    // factory method used to extend/override the creation of QAccessibleInterface for a widget
    QAccessibleInterface* factory(const QString& classname, QObject* object);
} // namespace ArrtAccesibility
