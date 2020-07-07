#include <QLabel>
#include <QVBoxLayout>
#include <View/Settings/VideoSettingsView.h>
#include <ViewModel/Settings/VideoSettingsModel.h>
#include <Widgets/FlatButton.h>

VideoSettingsView::VideoSettingsView(VideoSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
    , m_model(model)
{
    m_applyButton = new FlatButton(tr("Apply"), this);
    m_applyButton->setToolTip(tr("Apply the video settings"), tr("Re-connect and apply the video settings"));
    QObject::connect(m_applyButton, &FlatButton::clicked, this, [this]() {
        m_model->applySettings();
    });
    m_resetButton = new FlatButton(tr("Reset"), this);
    m_resetButton->setToolTip(tr("Reset"), tr("Reset to the current settings, discarding the changes"));
    QObject::connect(m_resetButton, &FlatButton::clicked, this, [this]() {
        m_model->resetToCurrentSettings();
    });

    m_statusLayout->addWidget(m_applyButton);
    m_statusLayout->addWidget(m_resetButton);

    QObject::connect(m_model, &VideoSettingsModel::updateUi, this, [this]() {
        updateUi();
    });
    updateUi();
}

void VideoSettingsView::updateUi()
{
    const bool canApplyOrResetToCurrentSettings = m_model->canApplyOrResetToCurrentSettings();
    m_applyButton->setEnabled(canApplyOrResetToCurrentSettings);
    m_resetButton->setEnabled(canApplyOrResetToCurrentSettings);
    m_status->setText(canApplyOrResetToCurrentSettings ? tr("Applying video settings will force reconnection to runtime.") : QString());
    setStatusBarColor(m_model->isVideoFormatSupported() ? Qt::lightGray : Qt::red);
}
