#pragma once
#include <QWidget>

class FormControl;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QFrame;
class SettingsBaseModel;

// Base view for settings model.

class SettingsBaseView : public QWidget
{
public:
    SettingsBaseView(SettingsBaseModel* baseModel, QWidget* parent = nullptr);
    void setStatusBarColor(const QColor& color);

protected:
    SettingsBaseModel* const m_baseModel;

    QList<FormControl*> m_widgets;

    QHBoxLayout* m_topLayout = {};
    QVBoxLayout* m_listLayout = {};
    QHBoxLayout* m_statusLayout = {};

    QLabel* m_status = {};
    QFrame* m_statusBar = {};

private:
    void updateUi();
};
