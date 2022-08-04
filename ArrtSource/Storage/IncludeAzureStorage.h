#pragma once

#pragma warning(push)
#pragma warning(disable : 4996)
#include <cpprest/http_client.h>
#pragma warning(pop)

#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>

using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Storage;
using namespace Azure::Storage::Blobs;
using namespace Azure::Storage::Sas;
