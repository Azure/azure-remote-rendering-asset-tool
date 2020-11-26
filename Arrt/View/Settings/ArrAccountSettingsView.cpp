#include <QHBoxLayout>
#include <QLabel>
#include <View/ArrtStyle.h>
#include <View/Settings/ArrAccountSettingsView.h>
#include <ViewModel/Settings/ArrAccountSettingsModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/ReadOnlyText.h>

ArrAccountSettingsView::ArrAccountSettingsView(ArrAccountSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
    , m_model(model)
{
    m_retryButton = new FlatButton(tr("Retry"), this);
    m_retryButton->setToolTip(tr("Retry connecting"), tr("Try and connect again if the Azure Remote Rendering connection failed"));
    QObject::connect(m_retryButton, &FlatButton::clicked, this, [this]() {
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
    QString statusDesc = toString(status);
    switch (status)
    {
        case AccountConnectionStatus::Authenticated:
            setStatus(SettingsBaseView::OK, statusDesc);
            break;
        case AccountConnectionStatus::CheckingCredentials:
            setStatus(SettingsBaseView::INPROGRESS, statusDesc);
            break;
        case AccountConnectionStatus::InvalidCredentials:
        case AccountConnectionStatus::NotAuthenticated:
            setStatus(SettingsBaseView::ERROR, statusDesc);
            break;
    }
    m_retryButton->setEnabled(status == AccountConnectionStatus::InvalidCredentials);
}
