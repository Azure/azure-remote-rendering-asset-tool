#include <View/ArrtStyle.h>
#include <Widgets/ToolbarButton.h>

ToolbarButton::ToolbarButton(const QString& text, QWidget* parent)
    : FlatButton(text, parent)
{
    setFont(ArrtStyle::s_toolbarFont);
}

ToolbarButton::ToolbarButton(const QString& text, const QIcon& icon, QWidget* parent)
    : ToolbarButton(text, parent)
{
    setIcon(icon, true);
}
