#include <QStyleOptionToolButton>
#include <View/ArrtStyle.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>

FlatButton::FlatButton(const QString& text, QWidget* parent)
    : QToolButton(parent)
{
    setText(text);
    setAccessibleName(text);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumHeight(ArrtStyle::controlHeight());
}

void FlatButton::setIcon(const QIcon& icon, bool keepText)
{
    QToolButton::setIcon(icon);
    if (keepText)
    {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else
    {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}

void FlatButton::setText(const QString& text)
{
    QToolButton::setText(text);
    setToolTip(text);
    setAccessibleName(text);
}

void FlatButton::setToolTip(const QString& title, const QString& details)
{
    QToolButton::setToolTip(ArrtStyle::formatToolTip(title, details));
}

QSize FlatButton::sizeHint() const
{
    QSize s = QToolButton::sizeHint();
    // even with no text, uses the minimum size from the text.
    if (text().isEmpty())
    {
        // based on QToolButton::minimumSizeHint()
        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        QFontMetrics fm = fontMetrics();
        QSize textSize = fm.size(Qt::TextShowMnemonic, {});
        textSize.setWidth(textSize.width() + fm.horizontalAdvance(QLatin1Char(' ')) * 2);
        opt.rect.setSize(textSize);
        s = s.expandedTo(style()->sizeFromContents(QStyle::CT_ToolButton, &opt, textSize, this));
    }
    // minimum width to make it square
    s.setWidth(qMax(s.width(), s.height()));
    return s;
}


QSize FlatButton::minimumSizeHint() const
{
    return sizeHint();
}
