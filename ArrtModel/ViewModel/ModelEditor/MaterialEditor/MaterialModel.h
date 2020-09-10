#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QObject>

class ArrSessionManager;
class ParameterModel;

// Qt model class for a material. It is used to wrap all of the properties in an ARR material
// with Qt properties (Q_PROPERTY) so that Qt reflection can be used to bind them in ParameterModels
// (see subclass MaterialPBR)

class MaterialModel : public QObject
{
public:
    MaterialModel(ArrSessionManager* sessionManager, QObject* parent = nullptr)
        : QObject(parent)
        , m_sessionManager(sessionManager)
    {
    }
    const QList<ParameterModel*>& getControls() const { return m_controls; }
    void setMaterial(const RR::ApiHandle<RR::Material>& material) { m_material = material; }

protected:
    QList<ParameterModel*> m_controls;
    RR::ApiHandle<RR::Material> m_material = {};
    ArrSessionManager* const m_sessionManager;
};

// macro used to create a Q_PROPERTY around a
// getMaterial()->Get[PropertyName]() and getMaterial()->Set[PropertyName]()
#define ARRT_PROPERTY(t, name)                                              \
    Q_PROPERTY(t name READ get##name WRITE set##name)                       \
    t get##name() const                                                     \
    {                                                                       \
        if (const auto m = getMaterial())                                   \
        {                                                                   \
            return static_cast<t>(m->Get##name());                          \
        }                                                                   \
        return {};                                                          \
    }                                                                       \
    bool set##name(const t& value)                                          \
    {                                                                       \
        if (auto m = getMaterial())                                         \
        {                                                                   \
            try                                                             \
            {                                                               \
                m->Set##name(static_cast<decltype(m->Get##name())>(value)); \
            }                                                               \
            catch (...)                                                     \
            {                                                               \
                return false;                                               \
            }                                                               \
            return true;                                                    \
        }                                                                   \
        return false;                                                       \
    }
