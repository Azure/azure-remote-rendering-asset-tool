#pragma once

#include <QFrame>
#include <QWidget>

class LogModel;
class QTreeView;
class FlatButton;
class QTextEdit;

// panel for visualizing the main log list

class LogView : public QFrame
{
public:
    LogView(LogModel* model, QWidget* parent = {});

    static QIcon getIconFromMsgType(QtMsgType type);

private:
    LogModel* const m_model;
    FlatButton* m_buttonSimple = {};
    FlatButton* m_buttonDetailed = {};
    bool m_detailedView = false;
    QTreeView* m_logList = {};
    QTextEdit* m_textView = {};

    void setDetailedView(bool detailed, bool force = false);
};
