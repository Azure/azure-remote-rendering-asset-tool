#include <QHBoxLayout>
#include <View/ArrtStyle.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarButton.h>

Toolbar::Toolbar(QWidget* parent)
    : FormControl(parent)
{
    m_layout = new QHBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(10);
    m_layout->addStretch();
    setLayout(m_layout);
    setHeader("");
}

void Toolbar::addButton(ToolbarButton* button)
{
    m_layout->insertWidget(m_layout->count() - 1, button);
}
