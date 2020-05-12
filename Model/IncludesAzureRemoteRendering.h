#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wreorder"
#pragma warning(push)
#pragma warning(disable : 4100 4002)
#include <AzureRemoteRendering.h>
#include <RemoteRenderingExtensions.h>
#pragma warning(pop)
#pragma clang diagnostic pop
namespace RR = Microsoft::Azure::RemoteRendering;
#include <QMetaType>

Q_DECLARE_METATYPE(RR::Float2);
Q_DECLARE_METATYPE(RR::Float3);
Q_DECLARE_METATYPE(RR::Float4);
Q_DECLARE_METATYPE(RR::Color4);
Q_DECLARE_METATYPE(RR::ConnectionStatus);
Q_DECLARE_METATYPE(RR::PbrMaterialFeatures);
Q_DECLARE_METATYPE(RR::Result);
Q_DECLARE_METATYPE(RR::ApiHandle<RR::Material>);
Q_DECLARE_METATYPE(RR::ApiHandle<RR::Entity>);
