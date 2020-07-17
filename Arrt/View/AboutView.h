#pragma once
#include <Widgets/SimpleMessageBox.h>

class AboutModel;

// "About ARRT" Dialog, used to show information on ARRT and its version

class AboutView : public SimpleMessageBox
{
public:
    AboutView(AboutModel* model, QWidget* parent = {});

private:
    AboutModel* m_model;
};
