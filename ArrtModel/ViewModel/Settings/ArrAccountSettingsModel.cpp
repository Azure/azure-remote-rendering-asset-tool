#include <Model/ArrFrontend.h>
#include <Model/Settings/ArrAccountSettings.h>
#include <ViewModel/Parameters/ComboBoxModel.h>
#include <ViewModel/Parameters/TextModel.h>
#include <ViewModel/Settings/ArrAccountSettingsModel.h>
#include <string_view>

Q_DECLARE_METATYPE(std::string);

ArrAccountSettingsModel::ArrAccountSettingsModel(ArrAccountSettings* arrAccountSettings, ArrFrontend* frontend, QObject* parent)
    : SettingsBaseModel(parent)
    , m_arrAccountSettings(arrAccountSettings)
    , m_frontend(frontend)
{
    using namespace std::literals;

    addControl(new TextModel(tr("ID"), m_arrAccountSettings, "id"sv, true));
    addControl(new TextModel(tr("Key"), m_arrAccountSettings, "key"sv, true, true));

    auto* accountDomainModel = new ComboBoxModelFromMap(tr("Account Domain"), m_arrAccountSettings, "accountDomain"sv);
    for (auto&& accountDomain : m_arrAccountSettings->getSupportedAccountDomains())
    {
        accountDomainModel->addEntry(accountDomain.m_label, QVariant::fromValue(accountDomain.m_accountDomain));
    }
    addControl(accountDomainModel);

    auto* regionModel = new ComboBoxModelFromMap(tr("Region"), m_arrAccountSettings, "region"sv);
    for (auto&& region : m_arrAccountSettings->getAvailableRegions())
    {
        // TODO This does not have to be a vector....
        regionModel->addEntry(region.m_label, QVariant::fromValue(region.m_domainUrl));
    }
    addControl(regionModel);

    connect(m_frontend, &ArrFrontend::onStatusChanged, this, [this]() {
        Q_EMIT updateUi();
    });

    auto connectAccount = [this]() {
        m_frontend->connectAccount(
            m_arrAccountSettings->getId().toStdString().c_str(),
            m_arrAccountSettings->getKey().toStdString().c_str(),
            m_arrAccountSettings->getAccountDomain().toStdString().c_str(),
            m_arrAccountSettings->getRegion().c_str());
    };
    QObject::connect(m_arrAccountSettings, &ArrAccountSettings::changed, this, connectAccount);
    connectAccount();
}

bool ArrAccountSettingsModel::isEnabled() const
{
    return getStatus() != AccountConnectionStatus::CheckingCredentials;
}

AccountConnectionStatus ArrAccountSettingsModel::getStatus() const
{
    return m_frontend->getStatus();
}

void ArrAccountSettingsModel::reconnectAccount()
{
    m_frontend->reconnectAccount();
}
