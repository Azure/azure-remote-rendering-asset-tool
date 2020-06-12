#pragma once
#include <QListView>

// Custom list view used by DirectoryButton to show and select its sub directories

class PopupList : public QListView
{
    Q_OBJECT
public:
    PopupList(QAbstractItemModel* model, QWidget* parent = {});

    virtual void focusOutEvent(QFocusEvent*) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void selected(QString item);

private:
    void finished();
    void adjustHeight();
};
