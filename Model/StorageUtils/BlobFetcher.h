#pragma once
#include <Model/StorageUtils/SegmentedAsyncFetcher.h>

// Async fetcher for blobs, it can be used just in a directory, or it can list all of the blobs

class BlobFetcher : public SegmentedAsyncFecther<azure::storage::list_blob_item_segment>
{
public:
    static std::shared_ptr<BlobFetcher> startFetching(
        const QObject* context, azure::storage::cloud_blob_container container, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback, std::function<void()> endCallback = {});
    static std::shared_ptr<BlobFetcher> startFetching(
        const QObject* context, azure::storage::cloud_blob_container container, utility::string_t directory, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback, std::function<void()> endCallback = {});

private:
    azure::storage::cloud_blob_container m_container;
    static const int s_maxNumberOfBlobsPerSegment = 100;

    utility::string_t m_directory;
    bool m_justBlobsInDirectory = false;

    BlobFetcher(const QObject* context, azure::storage::cloud_blob_container container, Callback callback, std::function<void()> endCallback);

    // set the directory and sets if it needs to list just blobs/dirs in the directory or all of the files in the subtree
    void setDirectoryFilter(utility::string_t directory, bool justBlobsInDirectory = false);

    pplx::task<azure::storage::list_blob_item_segment> doStartFetch();
    pplx::task<azure::storage::list_blob_item_segment> doProcess(const azure::storage::continuation_token& token);
};
