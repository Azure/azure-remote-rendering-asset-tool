#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/LineEditReturnOnFocusLost.h>

BlobContainerSelector::BlobContainerSelector(BlobContainerSelectorModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_selector(new QComboBox(this))
{
    setContentsMargins(0, 0, 0, 0);
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    l->addWidget(m_selector, 1);
    if (m_model->canNavigateToNewContainers())
    {
        m_addButton = new FlatButton("", this);
        m_addButton->setIcon(ArrtStyle::s_newfolderIcon);

        m_lineEdit = new LineEditReturnOnFocusLost(this);

        l->addWidget(m_lineEdit, 1);
        l->addWidget(m_addButton, 0);
        m_lineEdit->setVisible(false);

        connect(m_addButton, &FlatButton::pressed, this, [this]() {
            m_selector->setVisible(false);
            m_lineEdit->setVisible(true);
            m_lineEdit->setFocus();
        });

        connect(m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
            if (m_lineEdit->isVisible() && !m_lineEdit->text().isEmpty())
            {
                m_model->navigateToNewContainer(m_lineEdit->text());
                m_lineEdit->clear();
            }
            m_lineEdit->setVisible(false);
            m_selector->setVisible(true);
        });
    }

    m_selector->setModel(m_model->getAvailableContainersModel());

    auto onModelContainerChanged = [this]() {
        m_selector->setCurrentText(m_model->getCurrentContainer());
    };
    connect(model, &BlobContainerSelectorModel::currentContainerChanged, this, onModelContainerChanged);
    onModelContainerChanged();

    auto onIndexChanged = [this](const QString& container) {
        m_model->setCurrentContainer(container);
    };
    connect(m_selector, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, onIndexChanged);
}
