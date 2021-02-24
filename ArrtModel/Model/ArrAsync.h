#pragma once
#include <Model/IncludesAzureRemoteRendering.h>


template <typename T>
class ArrAsyncStatus
{
public:
    const RR::Status getStatus() const
    {
        return m_status;
    }

    const T& getResult() const
    {
        return m_result;
    }

    inline bool isRunning() const
    {
        return m_isRunning;
    }

    inline bool isCompleted() const 
    {
        return m_isCompleted;
    }

    inline bool isFaulted() const
    {
        return m_status != RR::Status::OK;
    }

protected:
    void setResult(RR::Status status, const T& res)
    {
        m_status = status;
        m_result = res;
        m_isRunning = false;
        m_isCompleted = true;
    }

    std::atomic_bool m_isCompleted = false;
    std::atomic_bool m_isRunning = false;
    RR::Status m_status = RR::Status::InProgress;
    T m_result = {};
};


class ArrPerformanceAssessmentAsync : public ArrAsyncStatus<RR::PerformanceAssessment>
{
public:
    void queryServerPerformanceAssessmentAsync(RR::ApiHandle<RR::RenderingSession> session)
    {
        m_isCompleted = false;
        m_isRunning = true;
        m_status = RR::Status::InProgress;
        session->Connection()->QueryServerPerformanceAssessmentAsync([this](RR::Status status, RR::PerformanceAssessment result) {
            this->setResult(status, result);
        });
    }
};

/*
class ArrSessionPropertiesArrayAsync : public ArrAsyncStatus<RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult>>
{
public:
    void getCurrentRenderingSessionsAsync(RR::ApiHandle<RR::RemoteRenderingClient> client, std::function<void()> completedCB)
    {
        m_isCompleted = false;
        m_isRunning = true;
        m_status = RR::Status::InProgress;
        client->GetCurrentRenderingSessionsAsync([this, completedCB](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result) {
            this->setResult(status, result);
            completedCB();
        });
    }
};



class ArrCreateNewRenderingSessionAsync : public ArrAsyncStatus<RR::ApiHandle<RR::CreateRenderingSessionResult>>
{
public:
    void renewAsync(RR::ApiHandle<RR::RemoteRenderingClient> client)
    {
        m_isCompleted = false;
        m_isRunning = true;
        m_status = RR::Status::InProgress;
        client->GetCurrentRenderingSessionsAsync([this, completedCB](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result) {
            this->setResult(status, result);
            completedCB();
        });
    }
};
*/