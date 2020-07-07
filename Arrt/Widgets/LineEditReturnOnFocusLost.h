#pragma once
#include <QLineEdit>

// modified line edit which emits return pressed when focus is lost or when escape is pressed (it clears the field first)

class LineEditReturnOnFocusLost : public QLineEdit
{
public:
    using QLineEdit::QLineEdit;

protected:
    virtual void focusOutEvent(QFocusEvent*) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
};