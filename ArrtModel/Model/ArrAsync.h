#pragma once
#include <Model/IncludesAzureRemoteRendering.h>

// A simple helper class to wrap an ARR async function and its result.
// This class is specifically helpful if you want to poll its result every frame

template <typename T>
class ArrAsyncStatus
{
public:
    
    // Gets the status code after execution. During execution this is Status::InProgress.
    const RR::Status getStatus() const
    {
        return m_status;
    }

    // Returns the result of the operation after completion.
    const T& getResult() const
    {
        return m_result;
    }

    // Indicates whether the async operation is still running. Note that isRunning() and isCompleted() can return false at the same time.
    inline bool isRunning() const
    {
        return m_isRunning;
    }

    // Indicates whether the operation has completed (successfully or with an error). Note that isRunning() and isCompleted() can return false at the same time.
    inline bool isCompleted() const 
    {
        return m_isCompleted;
    }

    // Indicates whether operation has been completed with an error.
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

// Specialization of ArrAsyncStatus that wraps around session->Connection()->QueryServerPerformanceAssessmentAsync
class ArrPerformanceAssessmentAsync : public ArrAsyncStatus<RR::PerformanceAssessment>
{
public:
    // Performs the query asynchronously
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
