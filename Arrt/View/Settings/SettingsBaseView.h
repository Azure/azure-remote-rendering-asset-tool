#pragma once
#include <QWidget>

class FormControl;
class QVBoxLayout;
class QHBoxLayout;
class ReadOnlyText;
class QFrame;
class SettingsBaseModel;
class StatusIndicator;

// Base view for settings model.

class SettingsBaseView : public QWidget
{
public:
    enum Status
    {
        NEUTRAL,
        OK,
        INPROGRESS,
        ERROR
    };

    SettingsBaseView(SettingsBaseModel* baseModel, QWidget* parent = nullptr);
    void setStatus(Status status, QString description);

protected:
    SettingsBaseModel* const m_baseModel;

    QList<FormControl*> m_widgets;

    QHBoxLayout* m_topLayout = {};
    QVBoxLayout* m_listLayout = {};
    QHBoxLayout* m_statusLayout = {};

    ReadOnlyText* m_status = {};
    StatusIndicator* m_statusBar = {};

private:
    void updateUi();
};
