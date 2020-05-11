#include <QHBoxLayout>
#include <QLabel>
#include <View/ArrtStyle.h>
#include <View/Settings/ArrAccountSettingsView.h>
#include <ViewModel/Settings/ArrAccountSettingsModel.h>
#include <Widgets/FlatButton.h>

ArrAccountSettingsView::ArrAccountSettingsView(ArrAccountSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
    , m_model(model)
{
    m_retryButton = new FlatButton(tr("Retry"), this);
    m_retryButton->setToolTip(tr("Retry connecting"), tr("Try and connect again if the Azure Remote Rendering connection failed"));
    QObject::connect(m_retryButton, &FlatButton::pressed, this, [this]() {
        m_model->reconnectAccount();
    });
    m_statusLayout->addWidget(m_retryButton);

    QObject::connect(m_model, &ArrAccountSettingsModel::updateUi, this, [this]() {
        updateUi();
    });
    updateUi();
}

void ArrAccountSettingsView::updateUi()
{
    const auto status = m_model->getStatus();
    switch (status)
    {
        case AccountConnectionStatus::Connected:
            setStatusBarColor(ArrtStyle::s_connectedColor);
            break;
        case AccountConnectionStatus::Connecting:
            setStatusBarColor(ArrtStyle::s_connectingColor);
            break;
        case AccountConnectionStatus::FailedToConnect:
        case AccountConnectionStatus::Disconnected:
            setStatusBarColor(ArrtStyle::s_disconnectedColor);
            break;
    }
    m_status->setText(toString(status));
    m_retryButton->setEnabled(status == AccountConnectionStatus::FailedToConnect);
}
