#pragma once
#include <QWidget>

class FlatButton;
class QLineEdit;

// Control used to navigate to a non existing directory. It is a button which expands to a text edit control, to enter the directory

class NewFolderButton : public QWidget
{
    Q_OBJECT
public:
    NewFolderButton(QWidget* parent = {});

Q_SIGNALS:
    void newFolderRequested(QString folder);

private:
    FlatButton* m_addButton;
    QLineEdit* m_lineEdit;
};
