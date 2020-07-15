#pragma once
#include <QDialog>

class AboutModel;

// "About ARRT" Dialog, used to show information on ARRT and its version

class AboutView : public QDialog
{
public:
    AboutView(AboutModel* model, QWidget* parent = {});

private:
    AboutModel* m_model;
};
