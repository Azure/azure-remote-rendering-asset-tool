#include <Model/StorageUtils/SegmentedAsyncFetcher.h>
#include <QApplication>
#include <QThread>

namespace
{
    // workaround to catch exceptions happening in a pplx task.
    template <typename TaskType>
    void catchExceptions(TaskType&& task)
    {
        task.then([](const pplx::task<void>& previousTask) {
            try
            {
                // the task is already completed, so this won't block
                previousTask.wait();
            }
            catch (std::exception& e)
            {
                qCritical(e.what());
            }
        });
    }
} // namespace

template <typename SegmentType>
SegmentedAsyncFecther<SegmentType>::SegmentedAsyncFecther(const QObject* context, Callback callback, EndCallback endCallback, DoStartFetch doStartFetch, DoProcess doProcess)
    : m_context(context)
    , m_callback(std::move(callback))
    , m_endCallback(std::move(endCallback))
    , m_doStartFetch(std::move(doStartFetch))
    , m_doProcess(std::move(doProcess))
{
}

template <typename SegmentType>
SegmentedAsyncFecther<SegmentType>::~SegmentedAsyncFecther()
{
    assert(m_finished == true);
}

template <typename SegmentType>
void SegmentedAsyncFecther<SegmentType>::cancel()
{
    assert(QThread::currentThread() == QCoreApplication::instance()->thread());
    m_finished = true;
}

template <typename SegmentType>
void SegmentedAsyncFecther<SegmentType>::startFetch()
{
    using namespace azure::storage;
    auto thisPtr = this->shared_from_this();
    continuation_token token;
    catchExceptions(m_doStartFetch().then([thisPtr](const SegmentType& segment) {
        thisPtr->process(segment);
    }));
}

template <typename SegmentType>
void SegmentedAsyncFecther<SegmentType>::process(const SegmentType& segment)
{
    // copy the segment because it needs to be queued
    QMetaObject::invokeMethod(QApplication::instance(), [thisPtr = this->shared_from_this(), segment]() {
        if (!thisPtr->m_context || thisPtr->m_finished)
        {
            return;
        }
        thisPtr->m_callback(segment);

        if (!segment.continuation_token().empty())
        {
            catchExceptions(thisPtr->m_doProcess(segment.continuation_token()).then([thisPtr](const SegmentType& segment) {
                thisPtr->process(segment);
            }));
        }
        else
        {
            thisPtr->m_finished = true;
            if (thisPtr->m_endCallback)
            {
                thisPtr->m_endCallback();
            }
        }
    });
}

template class SegmentedAsyncFecther<azure::storage::list_blob_item_segment>;
template class SegmentedAsyncFecther<azure::storage::container_result_segment>;
