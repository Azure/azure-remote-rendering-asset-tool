#pragma once
#include <QWidget>

class DirectoryButton;
class DirectoryProvider;
class QHBoxLayout;

// widget used to display a directory, as a bread crumbs control, with a button to navigate to a non existing folder

class DirectorySelector : public QWidget
{
    Q_OBJECT
public:
    DirectorySelector(DirectoryProvider* directoryModel, QWidget* parent = nullptr);

private:
    DirectoryProvider* const m_directoryModel;
    QList<DirectoryButton*> m_buttons;
    QHBoxLayout* m_buttonLayout = {};

    void updateUi();
};
