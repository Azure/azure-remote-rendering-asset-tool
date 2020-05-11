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
        for (auto&& res : m_sessionManager->loadedModel()->GetLoadedResourceOfType(RR::ObjectType::Material))
        {
            auto material = std::static_pointer_cast<RR::Material>(res);
            auto* newItem = new QStandardItem(QString::fromUtf8(material->Name().c_str()));
            newItem->setData(QVariant::fromValue(material), OBJECT_MATERIAL_ROLE);
            appendRow(newItem);
        }
    }
}

void MaterialFilteredListModel::filterBasedOnEntities(RR::RemoteManager* /*client*/, const QList<std::shared_ptr<RR::Entity>>& entityIds)
{
    m_materials.clear();

    for (const std::shared_ptr<RR::Entity>& entity : entityIds)
    {
        if (!entity->Valid())
        {
            qCritical() << tr("Invalid entity ") << entity->Handle();
        }
        else
        {
            //find all of the materials

            for (auto&& component : entity->Components())
            {
                if (component->Type() == RR::ObjectType::MeshComponent)
                {
                    const std::shared_ptr<RR::MeshComponent> meshComponent = std::static_pointer_cast<RR::MeshComponent>(component);
                    for (auto&& material : meshComponent->UsedMaterials())
                    {
                        if (material->Valid())
                        {
                            m_materials.insert(material->Handle());
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

const QSet<unsigned long long>& MaterialFilteredListModel::getFilteredMaterialSet() const
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
    const auto material = index.data(MaterialListModel::OBJECT_MATERIAL_ROLE).value<std::shared_ptr<RR::Material>>();
    return m_materials.contains(material->Handle());
}
