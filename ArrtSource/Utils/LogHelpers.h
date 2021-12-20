#pragma once

#include <QCoreApplication>
#include <QLoggingCategory>
#include <Rendering/IncludeAzureRemoteRendering.h>

// category types

namespace LoggingCategory
{
    extern QLoggingCategory ArrSdk;
    extern QLoggingCategory RenderingSession;
    extern QLoggingCategory AzureStorage;
}; // namespace LoggingCategory

/// Support logging of custom types.

QDebug& operator<<(QDebug& logger, const std::wstring& str);

QDebug& operator<<(QDebug& logger, const std::string& str);

QDebug& operator<<(QDebug& logger, const RR::Result& arrResult);

QDebug& operator<<(QDebug& logger, const RR::Status& arrStatus);

QDebug& operator<<(QDebug& logger, const RR::SessionGeneralContext& context);

QDebug& operator<<(QDebug& logger, const RR::RenderingSessionCreationOptions& info);

QDebug& operator<<(QDebug& logger, const RR::ConnectionStatus& connectionStatus);

QDebug& operator<<(QDebug& logger, const RR::RenderingSessionProperties& properties);

QString toString(const RR::Status& arrStatus);

QString toString(const RR::RenderingSessionStatus& sessionStatus);

/// Wrapper for ARR SDK log messages
void qArrSdkMessage(RR::LogLevel logLevel, const void* msgVoid);
