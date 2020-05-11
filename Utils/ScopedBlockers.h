#pragma once

class QWidget;
class QObject;

/// A helper class for working with QWidgets, that calls setUpdatesEnabled(false) on all given widgets and restores proper previous the state upon leaving the scope.
class ScopedUpdatesDisabled
{
public:
    ScopedUpdatesDisabled(
        QWidget* widget1,
        QWidget* widget2 = nullptr,
        QWidget* widget3 = nullptr,
        QWidget* widget4 = nullptr,
        QWidget* widget5 = nullptr,
        QWidget* widget6 = nullptr);
    ~ScopedUpdatesDisabled();

private:
    QWidget* m_widgets[6];
};

/// A helper class for working with QObjects that calls blockSignals(true) on all given objects and restores the proper previous state upon leaving the scope.
class ScopedBlockSignals
{
public:
    ScopedBlockSignals(
        QObject* object1,
        QObject* object2 = nullptr,
        QObject* object3 = nullptr,
        QObject* object4 = nullptr,
        QObject* object5 = nullptr,
        QObject* object6 = nullptr,
        QObject* object7 = nullptr,
        QObject* object8 = nullptr);
    ~ScopedBlockSignals();

private:
    QObject* m_objects[8];
};
