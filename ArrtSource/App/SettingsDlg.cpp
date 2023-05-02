#include "SettingsDlg.h"
#include <QCoreApplication>
#include <QDir>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardPaths>
#include <Rendering/ArrAccount.h>
#include <Storage/StorageAccount.h>

SettingsDlg::SettingsDlg(StorageAccount* storage, ArrAccount* arrClient, QWidget* parent)
    : QDialog(parent)
{
    m_storageAccount = storage;
    m_arrClient = arrClient;

    setupUi(this);

    LoadProfiles();

    if (!m_profiles.empty())
    {
        m_activeProfile = "unknown";

        for (auto it : m_profiles)
        {
            const ConnectionProfile& prof = it.second;

            if (prof.m_arrAccountDomain == m_arrClient->GetAccountDomain() &&
                prof.m_arrAccountId == m_arrClient->GetAccountId() &&
                prof.m_arrAccountKey == m_arrClient->GetAccountKey() &&
                prof.m_arrRegion == m_arrClient->GetRegion() &&
                prof.m_storageAccountKey == m_storageAccount->GetAccountKey() &&
                prof.m_storageAccountName == m_storageAccount->GetAccountName() &&
                prof.m_storageEndpointUrl == m_storageAccount->GetEndpointUrl())
            {
                m_activeProfile = it.first;
                break;
            }
        }
    }

    if (m_profiles.empty() || m_activeProfile == "unknown")
    {
        ConnectionProfile& profile = m_profiles[m_activeProfile];
        profile.m_arrAccountDomain = m_arrClient->GetAccountDomain();
        profile.m_arrAccountId = m_arrClient->GetAccountId();
        profile.m_arrAccountKey = m_arrClient->GetAccountKey();
        profile.m_arrRegion = m_arrClient->GetRegion();
        profile.m_storageAccountKey = m_storageAccount->GetAccountKey();
        profile.m_storageAccountName = m_storageAccount->GetAccountName();
        profile.m_storageEndpointUrl = m_storageAccount->GetEndpointUrl();
    }

    FillProfiles();
    PushProfile();
    SetActiveProfile(m_activeProfile);

    QPushButton* closeButton = Buttons->button(QDialogButtonBox::Close);
    closeButton->setAutoDefault(false);
    closeButton->setDefault(false);
}

SettingsDlg::~SettingsDlg() = default;

void SettingsDlg::closeEvent(QCloseEvent*)
{
    PullProfile();
    SaveProfiles();
    ApplyArr();
    ApplyStorage();
}

void SettingsDlg::on_Buttons_rejected()
{
    closeEvent(nullptr);
    accept();
}

void SettingsDlg::on_TestArr_clicked()
{
    ApplyArr();

    if (ArrAccountID->text().isEmpty())
    {
        QMessageBox::warning(this, "Test ARR account connection", "Please provide a valid account ID.", QMessageBox::Ok);
        return;
    }

    if (ArrAccountKey->text().isEmpty())
    {
        QMessageBox::warning(this, "Test ARR account connection", "Please provide a valid account key.", QMessageBox::Ok);
        return;
    }

    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        QCoreApplication::processEvents();

        switch (m_arrClient->GetConnectionStatus())
        {
            case ArrConnectionStatus::Authenticated:
                QMessageBox::information(this, "Test ARR account connection", "Connecting to ARR account succeeded.", QMessageBox::Ok);
                return;

            case ArrConnectionStatus::NotAuthenticated:
                QMessageBox::warning(this, "Test ARR account connection", "Connecting to ARR account failed.\n\nUser authentication failed.", QMessageBox::Ok);
                return;

            case ArrConnectionStatus::InvalidCredentials:
                QMessageBox::warning(this, "Test ARR account connection", "Connecting to ARR account failed.\n\nThe provided credentials are invalid.", QMessageBox::Ok);
                return;

            case ArrConnectionStatus::CheckingCredentials:
                break;
        }
    }

    QMessageBox::warning(this, "Test ARR account connection", "Test timed out - credentials are most likely incorrect.", QMessageBox::Ok);
}

void SettingsDlg::on_TestStorage_clicked()
{
    ApplyStorage();

    if (StorageName->text().isEmpty())
    {
        QMessageBox::warning(this, "Test storage account connection", "Please provide a valid storage name.", QMessageBox::Ok);
        return;
    }

    if (StorageKey->text().isEmpty())
    {
        QMessageBox::warning(this, "Test storage account connection", "Please provide a valid storage key.", QMessageBox::Ok);
        return;
    }

    if (StorageEndpoint->text().isEmpty())
    {
        QMessageBox::warning(this, "Test storage account connection", "Please provide a valid storage endpoint.", QMessageBox::Ok);
        return;
    }

    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        QCoreApplication::processEvents();

        switch (m_storageAccount->GetConnectionStatus())
        {
            case StorageConnectionStatus::Authenticated:
                QMessageBox::information(this, "Test storage account connection", "Connecting to storage account succeeded.", QMessageBox::Ok);
                return;

            case StorageConnectionStatus::NotAuthenticated:
                QMessageBox::warning(this, "Test storage account connection", "Connecting to storage account failed.\n\nUser authentication failed.", QMessageBox::Ok);
                return;

            case StorageConnectionStatus::InvalidCredentials:
                QMessageBox::warning(this, "Test storage account connection", "Connecting to storage account failed.\n\nThe provided credentials are invalid.", QMessageBox::Ok);
                return;

            case StorageConnectionStatus::CheckingCredentials:
                break;
        }
    }

    QMessageBox::warning(this, "Test storage account connection", "Test timed out - credentials are most likely incorrect.", QMessageBox::Ok);
}

void SettingsDlg::on_AddProfile_clicked()
{
    QString name = QInputDialog::getText(this, "Add Profile", "Name");
    if (name.isEmpty())
        return;

    if (m_profiles.find(name) != m_profiles.end())
    {
        QMessageBox::information(this, "Add Profile", "A profile named '" + name + "' already exists.", QMessageBox::Ok);
        return;
    }

    // copy the current profile settings, so that it is easier to reuse certain settings, like the storage account
    m_profiles[name] = m_profiles[m_activeProfile];
    SetActiveProfile(name);
}

void SettingsDlg::on_RemoveProfile_clicked()
{
    if (m_activeProfile == "<default>")
    {
        QMessageBox::information(this, "Remove Profile", "The default profile can't be removed.", QMessageBox::Ok);
        return;
    }

    m_profiles.erase(m_activeProfile);
    SetActiveProfile("<default>");
}

void SettingsDlg::on_ProfileDropDown_currentIndexChanged(int index)
{
    PullProfile();

    m_activeProfile = ProfileDropDown->itemText(index);

    PushProfile();
}

void SettingsDlg::ApplyArr()
{
    m_arrClient->SetSettings(ArrAccountID->text(), ArrAccountKey->text(), ArrAccountDomain->currentData().toString(), ArrRegion->currentData().toString());
    m_arrClient->ConnectToArrAccount();
}

void SettingsDlg::ApplyStorage()
{
    m_storageAccount->SetSettings(StorageName->text(), StorageKey->text(), StorageEndpoint->text());
    m_storageAccount->ConnectToStorageAccount();
}

void SettingsDlg::FillProfiles()
{
    if (ProfileDropDown->count() == m_profiles.size())
        return;

    ProfileDropDown->blockSignals(true);

    ProfileDropDown->clear();

    for (auto it : m_profiles)
    {
        ProfileDropDown->addItem(it.first);
    }

    ProfileDropDown->setCurrentIndex(-1);
    ProfileDropDown->blockSignals(false);
}

void SettingsDlg::SetActiveProfile(const QString& name)
{
    PullProfile();
    FillProfiles();

    m_activeProfile = name;
    int idx = ProfileDropDown->findText(m_activeProfile);
    if (idx < 0)
    {
        m_activeProfile = "<default>";
        idx = ProfileDropDown->findText(m_activeProfile);
    }


    ProfileDropDown->blockSignals(true);
    ProfileDropDown->setCurrentIndex(idx);
    ProfileDropDown->blockSignals(false);

    PushProfile();
}

void SettingsDlg::PullProfile()
{
    if (m_profiles.find(m_activeProfile) == m_profiles.end())
    {
        // profile was deleted
        return;
    }

    ConnectionProfile& profile = m_profiles[m_activeProfile];
    profile.m_arrAccountDomain = ArrAccountDomain->currentData().toString();
    profile.m_arrAccountId = ArrAccountID->text();
    profile.m_arrAccountKey = ArrAccountKey->text();
    profile.m_arrRegion = ArrRegion->currentData().toString();
    profile.m_storageAccountKey = StorageKey->text();
    profile.m_storageAccountName = StorageName->text();
    profile.m_storageEndpointUrl = StorageEndpoint->text();
}

void SettingsDlg::PushProfile()
{
    const ConnectionProfile& profile = m_profiles[m_activeProfile];

    // Storage Account
    {
        StorageName->setText(profile.m_storageAccountName);
        StorageKey->setText(profile.m_storageAccountKey);
        StorageEndpoint->setText(profile.m_storageEndpointUrl);
    }

    // ARR Account
    {
        ArrAccountID->setText(profile.m_arrAccountId);
        ArrAccountKey->setText(profile.m_arrAccountKey);

        // fill domain combobox
        {
            std::vector<ArrAccountDomainInfo> domains;
            m_arrClient->GetAvailableAccountDomains(domains);

            ArrAccountDomain->clear();

            int idx = -1;
            for (int i = 0; i < domains.size(); ++i)
            {
                const auto& domain = domains[i];
                ArrAccountDomain->addItem(domain.m_name, domain.m_url);

                if (profile.m_arrAccountDomain == domain.m_url)
                {
                    idx = i;
                }
            }

            ArrAccountDomain->setCurrentIndex(idx);
        }

        // fill region combobox
        {
            std::vector<ArrRegionInfo> regions;
            m_arrClient->GetAvailableRegions(regions);

            ArrRegion->clear();

            int idx = -1;
            for (int i = 0; i < regions.size(); ++i)
            {
                const auto& region = regions[i];
                ArrRegion->addItem(region.m_name, region.m_url);

                if (profile.m_arrRegion == region.m_url)
                {
                    idx = i;
                }
            }

            ArrRegion->setCurrentIndex(idx);
        }
    }
}

void SettingsDlg::SaveProfiles()
{
    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    appDataRootDir.mkpath(QString("."));
    QString configFile = appDataRootDir.absolutePath() + QDir::separator() + QString("ARR-profiles.json");

    QFile file(configFile);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonObject arrObj;
        {
            QJsonArray profilesArray;
            for (auto& it : m_profiles)
            {
                const ConnectionProfile& profile = it.second;

                QJsonObject obj;
                obj["name"] = it.first;
                obj["arr_account_id"] = profile.m_arrAccountId;
                obj["arr_account_key"] = profile.m_arrAccountKey;
                obj["arr_account_domain"] = profile.m_arrAccountDomain;
                obj["arr_region"] = profile.m_arrRegion;
                obj["storage_account_name"] = profile.m_storageAccountName;
                obj["storage_account_key"] = profile.m_storageAccountKey;
                obj["storage_endpoint_url"] = profile.m_storageEndpointUrl;

                profilesArray.append(obj);
            }
            arrObj["Profiles"] = profilesArray;
        }

        QJsonDocument saveDoc(arrObj);
        file.write(saveDoc.toJson());
    }
}

void SettingsDlg::LoadProfiles()
{
    m_profiles.clear();

    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    appDataRootDir.mkpath(QString("."));
    QString configFile = appDataRootDir.absolutePath() + QDir::separator() + QString("ARR-profiles.json");

    QFile file(configFile);

    if (file.open(QIODevice::ReadOnly))
    {
        const QByteArray fileContent = file.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(fileContent));

        const QJsonObject& rootObj = jsonDoc.object();

        QJsonArray profilesArray = rootObj["Profiles"].toArray();

        for (const auto& srcProfile : profilesArray)
        {
            QJsonObject obj = srcProfile.toObject();

            ConnectionProfile& dstProfile = m_profiles[obj["name"].toString()];
            dstProfile.m_arrAccountId = obj["arr_account_id"].toString();
            dstProfile.m_arrAccountKey = obj["arr_account_key"].toString();
            dstProfile.m_arrAccountDomain = obj["arr_account_domain"].toString();
            dstProfile.m_arrRegion = obj["arr_region"].toString();
            dstProfile.m_storageAccountName = obj["storage_account_name"].toString();
            dstProfile.m_storageAccountKey = obj["storage_account_key"].toString();
            dstProfile.m_storageEndpointUrl = obj["storage_endpoint_url"].toString();
        }
    }
}
