#pragma once
#include <QCoreApplication>

// Account connection status for ARR and Azure Blob accounts
enum class AccountConnectionStatus
{
    Connected,
    Disconnected,
    Connecting,
    FailedToConnect
};

inline QString toString(AccountConnectionStatus status)
{
    switch (status)
    {
        case AccountConnectionStatus::Connected:
            return QCoreApplication::tr("Connected");
        case AccountConnectionStatus::Disconnected:
            return QCoreApplication::tr("Disconnected");
        case AccountConnectionStatus::Connecting:
            return QCoreApplication::tr("Connecting");
        case AccountConnectionStatus::FailedToConnect:
            return QCoreApplication::tr("FailedToConnect");
    }
    return QCoreApplication::tr("Unknown");
}
