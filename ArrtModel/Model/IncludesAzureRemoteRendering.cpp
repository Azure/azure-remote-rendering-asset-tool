#include <Model/IncludesAzureRemoteRendering.h>

bool operator<(const RR::ApiHandle<RR::Entity>& lhs, const RR::ApiHandle<RR::Entity>& rhs)
{
    return lhs->GetHandle() < rhs->GetHandle();
}

uint qHash(const RR::ApiHandle<RR::Entity>& e)
{
    return static_cast<uint>(std::hash<unsigned long long>()(e ? e->GetHandle() : 0));
}

uint qHash(const RR::ApiHandle<RR::Material>& m)
{
    return static_cast<uint>(std::hash<unsigned long long>()(m && m->GetValid() ? m->GetHandle() : 0));
}
