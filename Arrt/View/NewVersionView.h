#pragma once
#include <Widgets/SimpleMessageBox.h>

class NewVersionModel;

// Dialog to show that there is a new version

class NewVersionView : public SimpleMessageBox
{
public:
    NewVersionView(NewVersionModel* model, QWidget* parent = {});

private:
    NewVersionModel* m_model;
};
