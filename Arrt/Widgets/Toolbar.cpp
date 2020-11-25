#include <QHBoxLayout>
#include <View/ArrtStyle.h>
#include <Widgets/FlowLayout.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarButton.h>

Toolbar::Toolbar(QWidget* parent)
    : FormControl(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    m_layout = new FlowLayout(nullptr, 0, 5, 5);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);
    setHeader("");
}

void Toolbar::addButton(ToolbarButton* button)
{
    m_layout->addWidget(button);
}
