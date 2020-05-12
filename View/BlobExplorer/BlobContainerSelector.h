#pragma once
#include <QComboBox>

class BlobContainerSelectorModel;

// combobox used for selecting a container, using BlobContainerSelectorModel

class BlobContainerSelector : public QComboBox
{
    Q_OBJECT
public:
    BlobContainerSelector(BlobContainerSelectorModel* model, QWidget* parent = {});

private:
    BlobContainerSelectorModel* const m_model;
};
