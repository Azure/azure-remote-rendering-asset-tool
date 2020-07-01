#pragma once

#include <QObject>

// Azure Storage account settings.

class AzureStorageAccountSettings : public QObject
{
    Q_OBJECT

private:
    Q_PROPERTY(QString name MEMBER m_name NOTIFY changed);
    Q_PROPERTY(QString key READ getKey WRITE setKey NOTIFY changed);
    Q_PROPERTY(QString blobEndpoint MEMBER m_blobEndpoint NOTIFY changed);

public:
    AzureStorageAccountSettings(QObject* parent);

    const QString& getName() const { return m_name; }
    QString getKey() const;
    bool setKey(const QString& key);
    const QString& getBlobEndpoint() const { return m_blobEndpoint; }

    void loadFromJson(const QJsonObject& storageAccountConfig);
    QJsonObject saveToJson() const;

Q_SIGNALS:
    void changed();

private:
    // name of the storage account
    QString m_name = {};

    // primary key of the storage account
    QString m_key = {};

    // blob endpoint of the storage account (e.g. https://[s.a. name].blob.core.windows.net/)
    QString m_blobEndpoint = {};
};
