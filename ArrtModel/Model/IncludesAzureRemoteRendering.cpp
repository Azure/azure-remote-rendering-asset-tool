#include <Model/IncludesAzureRemoteRendering.h>

bool operator<(const RR::ApiHandle<RR::Entity>& lhs, const RR::ApiHandle<RR::Entity>& rhs)
{
    return lhs->Handle() < rhs->Handle();
}

uint qHash(const RR::ApiHandle<RR::Entity>& e)
{
    return static_cast<uint>(std::hash<unsigned long long>()(e ? e->Handle() : 0));
}

uint qHash(const RR::ApiHandle<RR::Material>& m)
{
    return static_cast<uint>(std::hash<unsigned long long>()(m && *m->Valid() ? m->Handle() : 0));
}
