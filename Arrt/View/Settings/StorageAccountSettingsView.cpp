#include <QHBoxLayout>
#include <View/Settings/StorageAccountSettingsView.h>
#include <ViewModel/Settings/StorageAccountSettingsModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/ReadOnlyText.h>

StorageAccountSettingsView::StorageAccountSettingsView(StorageAccountSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
    , m_model(model)
{
    m_retryButton = new FlatButton(tr("Retry"), this);
    m_retryButton->setToolTip(tr("Retry authenticating"), tr("Try and authenticate again if the Azure Storage authentication failed with the account credentials"));
    QObject::connect(m_retryButton, &FlatButton::clicked, this, [this]() {
        m_model->reconnectAccount();
    });
    m_statusLayout->addWidget(m_retryButton);

    QObject::connect(m_model, &StorageAccountSettingsModel::updateUi, this, [this]() {
        updateUi();
    });
    updateUi();
}

void StorageAccountSettingsView::updateUi()
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
