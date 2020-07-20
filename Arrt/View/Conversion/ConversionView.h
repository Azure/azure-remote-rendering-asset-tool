#pragma once
#include <QWidget>

class ConversionModel;
class FlatButton;
class FlatButton;
class ReadOnlyText;
class QLabel;
class BoundWidget;
class QLineEdit;
class QComboBox;
class FormControl;

class ConversionView : public QWidget
{
    Q_OBJECT
public:
    ConversionView(ConversionModel* conversionModel, QWidget* parent = nullptr);

private:
    ConversionModel* m_model;
    QLineEdit* m_name = {};
    ReadOnlyText* m_status = {};
    ReadOnlyText* m_conversionId = {};
    FormControl* m_conversionIdControl = {};

    QLineEdit* m_inputLabel = {};
    QLineEdit* m_outputLabel = {};
    FlatButton* m_inputButton = {};
    FlatButton* m_outputButton = {};
    QComboBox* m_rootDirComboBox = {};

    FlatButton* m_startConversionButton = {};
    FlatButton* m_reingestButton = {};
    ReadOnlyText* m_uploadStatusLabel = {};
    QList<QWidget*> m_configControls;

    QPalette m_configLabelPalette;

    void updateUi();

    QLabel* createConfigLabel(const QString& text) const;
};
