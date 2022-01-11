#pragma once

#pragma warning(push)
#pragma warning(disable : 4996)
#include <cpprest/http_client.h>
#pragma warning(pop)

#if NEW_STORAGE_SDK

#    include <azure/core.hpp>
#    include <azure/storage/blobs.hpp>
#    include <azure/storage/blobs/blob_sas_builder.hpp>

using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Storage;
using namespace Azure::Storage::Blobs;
using namespace Azure::Storage::Sas;

#else

#    pragma warning(push)
#    pragma warning(disable : 4996)
#    include <cpprest/containerstream.h>
#    include <cpprest/filestream.h>
#    include <was/blob.h>
#    include <was/core.h>
#    include <was/queue.h>
#    include <was/storage_account.h>
#    pragma warning(pop)

using namespace azure::storage;

#endif