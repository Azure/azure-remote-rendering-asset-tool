#include <Widgets/LineEditReturnOnFocusLost.h>

#include <QKeyEvent>

void LineEditReturnOnFocusLost::focusOutEvent(QFocusEvent*)
{
    Q_EMIT returnPressed();
}

void LineEditReturnOnFocusLost::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            clear();
            Q_EMIT returnPressed();
            break;
        default:
            QLineEdit::keyPressEvent(event);
            break;
    }
}
