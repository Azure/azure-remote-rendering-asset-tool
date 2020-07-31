#pragma once
#include <QCoreApplication>

// Account connection status for ARR and Azure Blob accounts
enum class AccountConnectionStatus
{
    Authenticated,
    NotAuthenticated,
    CheckingCredentials,
    InvalidCredentials
};

inline QString toString(AccountConnectionStatus status)
{
    switch (status)
    {
        case AccountConnectionStatus::Authenticated:
            return QCoreApplication::tr("Authenticated");
        case AccountConnectionStatus::NotAuthenticated:
            return QCoreApplication::tr("Not authenticated");
        case AccountConnectionStatus::CheckingCredentials:
            return QCoreApplication::tr("Checking credentials");
        case AccountConnectionStatus::InvalidCredentials:
            return QCoreApplication::tr("Invalid credentials");
    }
    return QCoreApplication::tr("Unknown");
}
