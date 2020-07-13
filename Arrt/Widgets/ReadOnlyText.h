#pragma once
#include <QLineEdit>

// Control showing read-only text, which is still focusable/copyable and it's exposed for accessibility

class ReadOnlyText : public QLineEdit
{
    Q_OBJECT
public:
    ReadOnlyText(const QString& text, QWidget* parent = {});
    ReadOnlyText(QWidget* parent = {});
};
