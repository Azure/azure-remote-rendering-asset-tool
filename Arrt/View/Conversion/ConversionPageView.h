#pragma once
#include <QSplitter>

class ConversionPageModel;
class Navigator;
class FlatButton;
class QLabel;

// Main view for the conversion (it has the current conversion list and the selected conversion view)

class ConversionPageView : public QSplitter
{
public:
    ConversionPageView(ConversionPageModel* model, QWidget* parent = nullptr);

private:
    ConversionPageModel* const m_model;
    FlatButton* m_removeConversionButton = {};
    QLabel* m_description = {};

    void updateUi();
};
