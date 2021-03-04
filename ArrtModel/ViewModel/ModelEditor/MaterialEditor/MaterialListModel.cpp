#include <Model/ArrSessionManager.h>
#include <QDebug>
#include <ViewModel/ModelEditor/MaterialEditor/MaterialListModel.h>

MaterialListModel::MaterialListModel(ArrSessionManager* sessionManager, QObject* parent)
    : QStandardItemModel(parent)
    , m_sessionManager(sessionManager)
{
    QObject::connect(m_sessionManager, &ArrSessionManager::sessionAboutToChange, this, [this]() {
        clear();
    });
    QObject::connect(m_sessionManager, &ArrSessionManager::sessionChanged, this, [this]() {
        reset();
    });
    QObject::connect(m_sessionManager, &ArrSessionManager::rootIdChanged, this, [this]() {
        reset();
    });
    reset();
}

void MaterialListModel::reset()
{
    clear();
    if (auto loadedModel = m_sessionManager->loadedModel())
    {
        std::vector<RR::ApiHandle<RR::ResourceBase>> materials;
        if (loadedModel->GetLoadedResourceOfType(RR::ObjectType::Material, materials) == RR::Status::OK)
        {
            for (auto&& res : materials)
            {
                const RR::ApiHandle<RR::Material> material = res.as<RR::Material>();
                if (material && material->GetValid())
                {
                    std::string name;
                    material->GetName(name);
                    auto* newItem = new QStandardItem(QString::fromUtf8(name.c_str()));
                    newItem->setData(QVariant::fromValue(material), OBJECT_MATERIAL_ROLE);
                    appendRow(newItem);
                }
            }
        }
    }
}

void MaterialFilteredListModel::filterBasedOnEntities(const QList<RR::ApiHandle<RR::Entity>>& entityIds)
{
    m_materials.clear();

    for (const RR::ApiHandle<RR::Entity>& entity : entityIds)
    {
        if (!entity->GetValid())
        {
            qCritical() << tr("Invalid entity ") << entity->GetHandle();
        }
        else
        {
            //find all of the materials
            std::vector<RR::ApiHandle<RR::ComponentBase>> components;
            entity->GetComponents(components);
            for (auto&& component : components)
            {
                if (component->GetType() == RR::ObjectType::MeshComponent)
                {
                    const RR::ApiHandle<RR::MeshComponent> meshComponent = component.as<RR::MeshComponent>();
                    std::vector<RR::ApiHandle<RR::Material>> materials;
                    meshComponent->GetUsedMaterials(materials);
                    for (auto&& material : materials)
                    {
                        if (material->GetValid())
                        {
                            m_materials.insert(material);
                        }
                    }
                }
            }
        }
    }
    invalidateFilter();

    Q_EMIT filterChanged();
}

bool MaterialFilteredListModel::isFiltered() const
{
    return !m_materials.empty();
}

const QSet<RR::ApiHandle<RR::Material>>& MaterialFilteredListModel::getFilteredMaterialSet() const
{
    return m_materials;
}

bool MaterialFilteredListModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_materials.empty())
    {
        return true;
    }
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto material = index.data(MaterialListModel::OBJECT_MATERIAL_ROLE).value<RR::ApiHandle<RR::Material>>();
    return m_materials.contains(material);
}
