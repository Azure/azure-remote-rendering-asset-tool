#include <Model/ArrSessionManager.h>
#include <QDebug>
#include <ViewModel/ModelEditor/MaterialListModel.h>

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
    if (m_sessionManager->loadedModel())
    {
        if (auto materials = m_sessionManager->loadedModel()->GetLoadedResourceOfType(RR::ObjectType::Material))
        {
            for (auto&& res : materials.value())
            {
                auto material = res.as<RR::Material>();
                auto* newItem = new QStandardItem(QString::fromUtf8(material->Name()->c_str()));
                newItem->setData(QVariant::fromValue(material), OBJECT_MATERIAL_ROLE);
                appendRow(newItem);
            }
        }
    }
}

void MaterialFilteredListModel::filterBasedOnEntities(const QList<RR::ApiHandle<RR::Entity>>& entityIds)
{
    m_materials.clear();

    for (const RR::ApiHandle<RR::Entity>& entity : entityIds)
    {
        if (!entity->Valid())
        {
            qCritical() << tr("Invalid entity ") << entity->Handle();
        }
        else
        {
            //find all of the materials
            if (auto components = entity->Components())
            {
                for (auto&& component : components.value())
                {
                    if (*component->Type() == RR::ObjectType::MeshComponent)
                    {
                        const RR::ApiHandle<RR::MeshComponent> meshComponent = component.as<RR::MeshComponent>();
                        if (auto materials = meshComponent->UsedMaterials())
                        {
                            for (auto&& material : materials.value())
                            {
                                if (material->Valid().value())
                                {
                                    m_materials.insert(material);
                                }
                            }
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
