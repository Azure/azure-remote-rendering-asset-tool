#pragma once
#include <Model/IncludesAzureStorage.h>
#include <Model/StorageUtils/Cancellable.h>
#include <QPointer>

// class used to handle one asynchronous segmented blob list query (and handles cancellation)
// This object is not thread safe and meant to be used in the GUI thread.
// cancel and any smart pointer to the object should all be in the GUI thread. Also the callback will be called in the GUI thread

template <typename SegmentType>
class SegmentedAsyncFecther : public std::enable_shared_from_this<SegmentedAsyncFecther<SegmentType>>, public Cancellable
{
public:
    ~SegmentedAsyncFecther();

    typedef std::function<void(const SegmentType& segment)> Callback;
    typedef std::function<void()> EndCallback;

    typedef std::function<pplx::task<SegmentType>()> DoStartFetch;
    typedef std::function<pplx::task<SegmentType>(const azure::storage::continuation_token&)> DoProcess;


    // cancel the fetching. The callback won't be called anymore and the fetcher can be abandoned
    void cancel() override;

protected:
    SegmentedAsyncFecther(const QObject* context, Callback callback, EndCallback endCallback, DoStartFetch doStartFetch, DoProcess doProcess);

    void startFetch();
    void process(const SegmentType& segment);

private:
    // if the context object is dead, the callbacks are not executed anymore
    QPointer<const QObject> m_context;

    Callback m_callback;
    EndCallback m_endCallback;

    // the two implementations that are passed to the constructor
    DoStartFetch m_doStartFetch;
    DoProcess m_doProcess;

    // the fetching is finished: the callback won't be called anymore
    bool m_finished = false;
};
