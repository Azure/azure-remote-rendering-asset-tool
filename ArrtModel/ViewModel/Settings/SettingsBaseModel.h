#pragma once

#include <QObject>

class ParameterModel;

class SettingsBaseModel : public QObject
{
    Q_OBJECT

public:
    SettingsBaseModel(QObject* parent);

    const QList<ParameterModel*>& getControls() const { return m_controls; }

    virtual bool isEnabled() const = 0;

Q_SIGNALS:
    void updateUi();


protected:
    void addControl(ParameterModel* model);

private:
    QList<ParameterModel*> m_controls;
};
