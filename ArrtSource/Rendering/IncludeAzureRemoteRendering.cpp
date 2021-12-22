#include <Rendering/IncludeAzureRemoteRendering.h>

#pragma warning(push)
#pragma warning(disable : 4100 4189)


// THIS IS SUPER IMPORTANT!
// Every ARR project has to include this .inl file exactly once in some cpp-file in your app, otherwise you will get linker errors.
#include <AzureRemoteRendering.inl>


#pragma warning(disable : 4996)
#include <cpprest/http_client.h>

#pragma warning(pop)