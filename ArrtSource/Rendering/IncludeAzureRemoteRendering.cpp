#include <Rendering/IncludeAzureRemoteRendering.h>

// including this .inl file once somewhere in your apps source is crucial to not get linker errors for ARR

#pragma warning(push)
#pragma warning(disable : 4100 4189)
#include <AzureRemoteRendering.inl>
#pragma warning(disable : 4996)
#include <cpprest/http_client.h>
#pragma warning(pop)