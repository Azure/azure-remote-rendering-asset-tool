#pragma once

#include <QLoggingCategory>

#include <Rendering/IncludeAzureRemoteRendering.h>

namespace LoggingCategory
{
    extern QLoggingCategory ArrSdk;
    extern QLoggingCategory RenderingSession;
    extern QLoggingCategory AzureStorage;
}; // namespace LoggingCategory

/// Forwards ARR log messages to the Qt log
void ForwardArrLogMsgToQt(RR::LogLevel logLevel, const void* msg);

QString toString(const RR::Status& arrStatus);
QString toString(const RR::RenderingSessionStatus& sessionStatus);
QString toString(const RR::ConnectionStatus& connectionStatus);

QDebug& operator<<(QDebug& logger, const struct PrettyJson& prettyJson);
QDebug& operator<<(QDebug& logger, const RR::Result& arrResult);
QDebug& operator<<(QDebug& logger, const RR::Status& arrStatus);
QDebug& operator<<(QDebug& logger, const RR::SessionGeneralContext& context);
QDebug& operator<<(QDebug& logger, const RR::ConnectionStatus& connectionStatus);
