#include <Model/ArrSessionManager.h>
#include <Model/ModelEditor/MaterialPBR.h>
#include <Model/ModelEditor/MaterialProvider.h>
#include <QMetaType>
#include <ViewModel/Parameters/ComboBoxModel.h>

MaterialProvider::MaterialProvider(const Value<RR::ApiHandle<RR::Material>>* material, ArrSessionManager* sessionManager, QObject* parent)
    : QObject(parent)
    , m_sessionManager(sessionManager)
    , m_material(material)
    , m_emptyMaterial(new MaterialModel(sessionManager, this))
    , m_materialPBR(new MaterialPBR(sessionManager, this))
    , m_currentMaterial(m_emptyMaterial)
{
    QObject::connect(m_material, &Value<RR::ApiHandle<RR::Material>>::valueChanged, this, [this]() {
        updateControls();
    });
}

void MaterialProvider::updateControls()
{
    MaterialModel* m = m_emptyMaterial;


    if (auto material = m_material->get())
    {
        if (auto subType = material->MaterialSubType())
        {
            switch (subType.value())
            {
                case RR::MaterialType::Pbr:
                    m = m_materialPBR;
                    break;
                case RR::MaterialType::Color:
                    break;
            }
        }
    }
    m_currentMaterial = m;

    m_currentMaterial->setMaterial(m_material->get());
    materialChanged();
}

const QList<ParameterModel*>& MaterialProvider::getControls() const
{
    return m_currentMaterial->getControls();
}
