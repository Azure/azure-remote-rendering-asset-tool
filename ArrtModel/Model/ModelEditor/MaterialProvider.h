#pragma once

#include <Model/IncludesAzureRemoteRendering.h>
#include <QObject>
#include <Utils/Value.h>

class ArrSessionManager;
class MaterialModel;
class MaterialPBR;
class MaterialColor;
class ParameterModel;

// Model class used to expose the controls of the selected material.

class MaterialProvider : public QObject
{
    Q_OBJECT

public:
    MaterialProvider(const Value<RR::ApiHandle<RR::Material>>* material, ArrSessionManager* sessionManager, QObject* parent = nullptr);

    const QList<ParameterModel*>& getControls() const;

signals:
    void materialChanged();

private:
    ArrSessionManager* const m_sessionManager;
    const Value<RR::ApiHandle<RR::Material>>* const m_material;
    MaterialModel* const m_emptyMaterial;
    MaterialPBR* const m_materialPBR;
    MaterialColor* m_materialColor;
    /*MaterialCustom* m_materialCustom;*/

    MaterialModel* m_currentMaterial;

    void updateControls();
};
