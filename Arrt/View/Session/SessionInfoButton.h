#pragma once
#include <ViewModel/Session/SessionPanelModel.h>
#include <Widgets/FlatButton.h>

class SessionPanelModel;

// button which shows the status of the session. Clicking it will normally open a SessionInfoView

class SessionInfoButton : public FlatButton
{
public:
    SessionInfoButton(SessionPanelModel* model, QWidget* parent = {});

    static QString getStringFromStatus(SessionPanelModel::Status status);
    static QIcon getIconFromStatus(SessionPanelModel::Status status);

protected:
    virtual void paintEvent(QPaintEvent* e) override;
    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

private:
    QString getString() const;
    QString getTimeString() const;

    SessionPanelModel* const m_model;
};
