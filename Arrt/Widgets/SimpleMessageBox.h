#pragma once

#include <QDialog>
class QVBoxLayout;

// Simple styled message box with an OK button

class SimpleMessageBox : public QDialog
{
    Q_OBJECT
public:
    SimpleMessageBox(const QString& title, QWidget* parent = {});
    QVBoxLayout* getContentLayout() const { return m_contentLayout; }

private:
    QVBoxLayout* m_contentLayout;
};
