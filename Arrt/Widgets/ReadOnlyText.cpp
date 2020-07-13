#include <Widgets/ReadOnlyText.h>

ReadOnlyText::ReadOnlyText(const QString& text, QWidget* parent)
    : QLineEdit(parent)
{
    setText(text);
    setReadOnly(true);
    setStyleSheet("background:transparent");
}

ReadOnlyText::ReadOnlyText(QWidget* parent)
    : ReadOnlyText({}, parent)
{
}