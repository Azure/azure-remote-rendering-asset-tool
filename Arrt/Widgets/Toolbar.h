#pragma once
#include <Widgets/FormControl.h>

class ToolbarButton;
class QHBoxLayout;

// Group of ToolbarButtons in a horizontal layout

class Toolbar : public FormControl
{
public:
    Toolbar(QWidget* parent = {});
    void addButton(ToolbarButton* button);

private:
    QHBoxLayout* m_layout;
};
