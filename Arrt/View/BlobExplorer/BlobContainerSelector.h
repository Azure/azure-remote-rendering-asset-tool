#pragma once
#include <QWidget>

class BlobContainerSelectorModel;
class QComboBox;
class FlatButton;
class QLineEdit;

// combobox used for selecting a container, using BlobContainerSelectorModel

class BlobContainerSelector : public QWidget
{
    Q_OBJECT
public:
    BlobContainerSelector(BlobContainerSelectorModel* model, QWidget* parent = {});

private:
    BlobContainerSelectorModel* const m_model;
    QComboBox* const m_selector;
    FlatButton* m_addButton = {};
    QLineEdit* m_lineEdit = {};
};
