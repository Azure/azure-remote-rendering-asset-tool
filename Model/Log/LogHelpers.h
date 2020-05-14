#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

// category types

namespace LoggingCategory
{
    extern QLoggingCategory arrSdk;
    extern QLoggingCategory azureStorage;
    extern QLoggingCategory renderingSession;
    extern QLoggingCategory configuration;
    extern QLoggingCategory conversion;
}; // namespace LoggingCategory

/// Support logging of custom types.

inline QDebug& operator<<(QDebug& logger, const std::wstring& str)
{
    logger << QString::fromStdWString(str);
    return logger;
}

inline QDebug& operator<<(QDebug& logger, const std::string& str)
{
    logger << QString::fromStdString(str);
    return logger;
}

inline QDebug& operator<<(QDebug& logger, const RR::Result& arrResult)
{
    logger << RR::ResultToString(arrResult);
    return logger;
}

inline QDebug& operator<<(QDebug& logger, const RR::Status& arrStatus)
{
    if (arrStatus >= RR::Status::CoreReturnValueStart)
    {
        logger << RR::ResultToString((RR::Result)((uint32_t)arrStatus - 1 - (uint32_t)RR::Status::CoreReturnValueStart));
    }
    else
    {
#define logCase(val)        \
    case RR::Status::##val: \
        logger << #val;     \
        break;
        switch (arrStatus)
        {
            logCase(OK)
                logCase(Failed)
                    logCase(ObjectDisposed)
                        logCase(OutOfMemory)
                            logCase(InvalidArgument)
                                logCase(OutOfRange)
                                    logCase(NotImplemented)
                                        logCase(KeyNotFound)
        }
    }
#undef logCase
    return logger;
}

// Wrapper around QJsonObject to prettify output to QDebug.
// QJsonObject already has implementation of stream to QDebug which serialize object
// in non-indented format and with special characters escaping.
struct PrettyJson
{
    PrettyJson(const QJsonObject& jsonObj)
        : m_jsonObj(jsonObj)
    {
    }

    const QJsonObject& m_jsonObj;
};
inline QDebug& operator<<(QDebug& logger, const PrettyJson& prettyJson)
{
    QDebugStateSaver ss(logger);
    logger.noquote() << QJsonDocument(prettyJson.m_jsonObj).toJson(QJsonDocument::Indented);
    return logger;
}

inline QDebug& operator<<(QDebug& logger, const RR::SessionGeneralContext& context)
{
    QJsonObject contextInfo;
    contextInfo[QLatin1String("result")] = RR::ResultToString(context.Result);
    contextInfo[QLatin1String("http_response_code")] = (int)context.HttpResponseCode;
    contextInfo[QLatin1String("error_message")] = QString::fromStdString(context.ErrorMessage);
    contextInfo[QLatin1String("request_correlation_vector")] = QString::fromStdString(context.RequestCorrelationVector);
    contextInfo[QLatin1String("response_correlation_vector")] = QString::fromStdString(context.ResponseCorrelationVector);
    return logger << QCoreApplication::tr("Session context: ") << PrettyJson(contextInfo);
}

/// Wrapper for ARR sdk logging
inline void qArrSdkMessage(RR::LogLevel logLevel, const void* msgVoid)
{
    const char* msg = static_cast<const char*>(msgVoid);
    const auto& category(LoggingCategory::arrSdk);
    switch (logLevel)
    {
        case RR::LogLevel::Debug:
            qDebug(category, msg);
            break;
        case RR::LogLevel::Information:
            qInfo(category, msg);
            break;
        case RR::LogLevel::Warning:
            qWarning(category, msg);
            break;
        case RR::LogLevel::Error:
            qCritical(category, msg);
            break;
        case RR::LogLevel::None:
        case RR::LogLevel::Count:
            // nothing
            break;
    }
}
