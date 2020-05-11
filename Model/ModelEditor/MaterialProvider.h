#pragma once

#include <Model/IncludesAzureRemoteRendering.h>
#include <QObject>
#include <Utils/Value.h>

class ArrSessionManager;
class MaterialModel;
class MaterialPBR;
class ParameterModel;

// Model class used to expose the controls of the selected material.

class MaterialProvider : public QObject
{
    Q_OBJECT

public:
    MaterialProvider(const Value<std::shared_ptr<RR::Material>>* material, ArrSessionManager* sessionManager, QObject* parent = nullptr);

    const QList<ParameterModel*>& getControls() const;

signals:
    void materialChanged();

private:
    ArrSessionManager* const m_sessionManager;
    const Value<std::shared_ptr<RR::Material>>* const m_material;
    MaterialModel* const m_emptyMaterial;
    MaterialPBR* const m_materialPBR;
    /* QColorMaterial* m_materialPBR;
    QCustomMaterial* m_customMaterial;*/

    MaterialModel* m_currentMaterial;

    void updateControls();
};
