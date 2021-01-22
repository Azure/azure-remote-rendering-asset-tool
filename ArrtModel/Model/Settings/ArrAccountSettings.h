#pragma once

#include <QObject>

// ARR account settings.
// See Microsoft::Azure::RemoteRendering::Internal::ClientInit for the documentation on the ARR account parameters

class ArrAccountSettings : public QObject
{
    Q_OBJECT

public:
    // Arr region entry. m_label is visualized on the UI, and m_domainUrl is the url used for connection
    struct Region
    {
        QString m_label;
        QString m_domainUrl;
    };

    // Account Domain entry. m_label is visualized on the UI, and m_accountDomain is the account domain.
    struct AccountDomain
    {
        QString m_label;
        QString m_accountDomain;
    };

private:
    Q_PROPERTY(QString id MEMBER m_id NOTIFY changed);
    Q_PROPERTY(QString key READ getKey WRITE setKey);
    Q_PROPERTY(QString accountDomain MEMBER m_accountDomain NOTIFY changed);
    Q_PROPERTY(QString region MEMBER m_region NOTIFY changed);

public:
    ArrAccountSettings(QObject* parent);

    const QString& getId() const { return m_id; }
    QString getKey() const;
    bool setKey(const QString& key);
    QString getAccountDomain() const { return m_accountDomain; }
    std::string getRegion() const { return m_region.toStdString(); }

    // return the list of available arr regions
    std::vector<Region> getAvailableRegions() const;

    // return the list of supported Remote Rendering account domains
    std::vector<AccountDomain> getSupportedAccountDomains() const;

    void loadFromJson(const QJsonObject& arrAccountConfig);
    QJsonObject saveToJson() const;

Q_SIGNALS:
    void changed();

private:
    QString m_id = {};
    QString m_key = {};
    QString m_accountDomain = {};
    QString m_region = "westeurope.mixedreality.azure.com";
    std::map<QString, Region> m_availableRegionsMap;
    std::map<QString, AccountDomain> m_supportedAccountDomainsMap;
};
