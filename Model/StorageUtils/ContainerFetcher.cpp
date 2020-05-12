#include <Model/StorageUtils/ContainerFetcher.h>


ContainerFetcher::ContainerFetcher(const QObject* context, azure::storage::cloud_blob_client* client, Callback callback, std::function<void()> endCallback)
    : SegmentedAsyncFecther<azure::storage::container_result_segment>(
          context, std::move(callback), std::move(endCallback), [this]() { return doStartFetch(); }, [this](const azure::storage::continuation_token& token) { return doProcess(token); })
    , m_client(client)
{
}

std::shared_ptr<ContainerFetcher> ContainerFetcher::startFetching(
    const QObject* context, azure::storage::cloud_blob_client* client, std::function<void(const azure::storage::container_result_segment& segment)> callback, std::function<void()> endCallback)
{
    std::shared_ptr<ContainerFetcher> ptr(new ContainerFetcher(context, client, std::move(callback), std::move(endCallback)));
    ptr->startFetch();
    return ptr;
}

pplx::task<azure::storage::container_result_segment> ContainerFetcher::doStartFetch()
{
    azure::storage::continuation_token token;
    return m_client->list_containers_segmented_async(token);
}

pplx::task<azure::storage::container_result_segment> ContainerFetcher::doProcess(const azure::storage::continuation_token& token)
{
    return m_client->list_containers_segmented_async(token);
}
