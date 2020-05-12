#pragma once
#include <Model/StorageUtils/SegmentedAsyncFetcher.h>

// Async fetcher for the containers.

class ContainerFetcher : public SegmentedAsyncFecther<azure::storage::container_result_segment>
{
public:
    static std::shared_ptr<ContainerFetcher> startFetching(
        const QObject* context, azure::storage::cloud_blob_client* client, std::function<void(const azure::storage::container_result_segment& segment)> callback, std::function<void()> endCallback = {});

private:
    azure::storage::cloud_blob_client* m_client;
    ContainerFetcher(const QObject* context, azure::storage::cloud_blob_client* client, Callback callback, std::function<void()> endCallback);

    pplx::task<azure::storage::container_result_segment> doStartFetch();
    pplx::task<azure::storage::container_result_segment> doProcess(const azure::storage::continuation_token& token);
};
