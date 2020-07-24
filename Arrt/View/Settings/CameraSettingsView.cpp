#include <QVBoxLayout>
#include <View/Parameters/BoundFloatSlider.h>
#include <View/Parameters/BoundToggleButton.h>
#include <View/Settings/CameraSettingsView.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Settings/CameraSettingsModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>

CameraSettingsView::CameraSettingsView(CameraSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
{
    m_statusLayout->parentWidget()->setVisible(false);

    FormControl* w = new FormControl(this);
    m_listLayout->addWidget(w);
    w->setHeader(model->getGlobalScaleModel()->getName());
    auto* h = new QHBoxLayout();
    auto* scaleSlider = new BoundFloatSlider(model->getGlobalScaleModel(), w);
    h->addWidget(scaleSlider);

    auto* toggleButton = new BoundToggleButton(model->getAutoGlobalScaleModel());
    toggleButton->setToolTip(tr("Automatic global scale"), tr("Set the global scale automatically to a power of 10 dependent on the loaded model size"));
    h->addWidget(toggleButton);
    w->setLayout(h);

    auto updateButton = [this, scaleSlider, toggleButton, model]() {
        const bool val = toggleButton->isChecked();
        scaleSlider->setEnabled(!model->getAutoGlobalScaleModel()->getValue());
    };
    connect(toggleButton, &FlatButton::toggled, this, updateButton, Qt::QueuedConnection);
    updateButton();

    m_listLayout->addWidget(w);

    connect(model, &CameraSettingsModel::scaleChanged, this, [this, scaleSlider, toggleButton]() {
        scaleSlider->updateFromModel();
        toggleButton->updateFromModel();
    });
}
