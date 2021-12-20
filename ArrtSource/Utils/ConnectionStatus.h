#pragma once

#include <QString>

enum class AccountConnectionStatus
{
    Authenticated,
    NotAuthenticated,
    CheckingCredentials,
    InvalidCredentials
};

inline QString ToString(AccountConnectionStatus status)
{
    switch (status)
    {
        case AccountConnectionStatus::Authenticated:
            return "Authenticated";
        case AccountConnectionStatus::NotAuthenticated:
            return "Not authenticated";
        case AccountConnectionStatus::CheckingCredentials:
            return "Checking credentials";
        case AccountConnectionStatus::InvalidCredentials:
            return "Invalid credentials";
    }
    return "Unknown";
}
