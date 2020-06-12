#include <Model/StorageUtils/BlobFetcher.h>

std::shared_ptr<BlobFetcher> BlobFetcher::startFetching(
    const QObject* context, azure::storage::cloud_blob_container container, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback, std::function<void()> endCallback)
{
    std::shared_ptr<BlobFetcher> ptr(new BlobFetcher(context, std::move(container), std::move(callback), std::move(endCallback)));
    ptr->startFetch();
    return ptr;
}

std::shared_ptr<BlobFetcher> BlobFetcher::startFetching(
    const QObject* context, azure::storage::cloud_blob_container container, utility::string_t directory, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback, std::function<void()> endCallback)
{
    std::shared_ptr<BlobFetcher> ptr(new BlobFetcher(context, std::move(container), std::move(callback), std::move(endCallback)));
    ptr->setDirectoryFilter(std::move(directory), true);
    ptr->startFetch();
    return ptr;
}


BlobFetcher::BlobFetcher(const QObject* context, azure::storage::cloud_blob_container container, Callback callback, std::function<void()> endCallback)
    : SegmentedAsyncFecther<azure::storage::list_blob_item_segment>(
          context, std::move(callback), std::move(endCallback), [this]() { return doStartFetch(); }, [this](const azure::storage::continuation_token& token) { return doProcess(token); })
    , m_container(std::move(container))
{
}

void BlobFetcher::setDirectoryFilter(utility::string_t directory, bool justBlobsInDirectory)
{
    m_directory = std::move(directory);
    m_justBlobsInDirectory = justBlobsInDirectory;
}

pplx::task<azure::storage::list_blob_item_segment> BlobFetcher::doStartFetch()
{
    using namespace azure::storage;
    continuation_token token;
    return m_container.list_blobs_segmented_async(m_directory, !m_justBlobsInDirectory, blob_listing_details::none, s_maxNumberOfBlobsPerSegment, token, {}, {});
}

pplx::task<azure::storage::list_blob_item_segment> BlobFetcher::doProcess(const azure::storage::continuation_token& token)
{
    return m_container.list_blobs_segmented_async(m_directory, !m_justBlobsInDirectory, azure::storage::blob_listing_details::none, s_maxNumberOfBlobsPerSegment, token, {}, {});
}
