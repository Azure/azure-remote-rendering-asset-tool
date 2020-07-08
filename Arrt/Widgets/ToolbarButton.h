#pragma once
#include <Widgets/FlatButton.h>

// Button used for toolbar buttons in a panel

class ToolbarButton : public FlatButton
{
public:
    ToolbarButton(const QString& text, QWidget* parent = {});
	ToolbarButton(const QString& text, const QIcon& icon, QWidget* parent = {});
};
