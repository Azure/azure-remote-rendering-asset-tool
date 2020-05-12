#pragma once
#include <QToolButton.h>

// flat button, default button in ARRT.
// It doesn't have a border, unless it's toggled or hovered

class FlatButton : public QToolButton
{
public:
    FlatButton(const QString& text, QWidget* parent = {});

    // changes the icon in the button. If keepText is false, the text will be removed
    void setIcon(const QIcon& icon, bool keepText = false);

    // set the text, and affects also tooltip and accessible name
    void setText(const QString& text);

    virtual QSize minimumSizeHint() const override;

    using QToolButton::setToolTip;

    // set a formatted tool-tip for this button
    virtual void setToolTip(const QString& title, const QString& details);
};
