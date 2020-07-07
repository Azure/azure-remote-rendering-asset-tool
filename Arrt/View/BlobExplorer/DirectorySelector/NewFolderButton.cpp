#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/DirectorySelector/NewFolderButton.h>
#include <Widgets/FlatButton.h>
#include <Widgets/LineEditReturnOnFocusLost.h>

NewFolderButton::NewFolderButton(QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    m_addButton = new FlatButton("", this);
    m_addButton->setIcon(ArrtStyle::s_newfolderIcon);
    connect(m_addButton, &FlatButton::clicked, this, [this]() {
        m_lineEdit->setVisible(true);
        m_lineEdit->setFocus();
        m_addButton->setVisible(false);
    });
    m_addButton->setToolTip(tr("Add Sub-Directory"), tr("Add a directory and navigate to it. The directory is not persisted in Azure."));

    m_lineEdit = new LineEditReturnOnFocusLost(this);
    m_lineEdit->setVisible(false);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_lineEdit->isVisible() && !m_lineEdit->text().isEmpty())
        {
            Q_EMIT newFolderRequested(m_lineEdit->text());
            m_lineEdit->clear();
        }
        m_lineEdit->setVisible(false);
        m_addButton->setVisible(true);
    });

    l->addWidget(m_lineEdit);
    l->addWidget(m_addButton);
}
