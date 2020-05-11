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
    void setMaterial(const std::shared_ptr<RR::Material>& material) { m_material = material; }

protected:
    QList<ParameterModel*> m_controls;
    std::shared_ptr<RR::Material> m_material = {};
    ArrSessionManager* const m_sessionManager;
};

// macro used to create a Q_PROPERTY around a
// getMaterial()->Get[PropertyName]() and getMaterial()->Set[PropertyName]()
#define ARRT_PROPERTY(type, name)                                 \
    Q_PROPERTY(type name READ get##name WRITE set##name)          \
    type get##name() const                                        \
    {                                                             \
        if (const RR::PbrMaterial* m = getMaterial())             \
        {                                                         \
            return static_cast<type>(m->name());                  \
        }                                                         \
        return {};                                                \
    }                                                             \
    bool set##name(const type& value)                             \
    {                                                             \
        if (RR::PbrMaterial* m = getMaterial())                   \
        {                                                         \
            try                                                   \
            {                                                     \
                m->name(static_cast<decltype(m->name())>(value)); \
            }                                                     \
            catch (...)                                           \
            {                                                     \
                return false;                                     \
            }                                                     \
            return true;                                          \
        }                                                         \
        return false;                                             \
    }
