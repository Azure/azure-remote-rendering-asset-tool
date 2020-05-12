#include <Model/ArrSessionManager.h>
#include <Model/ModelEditor/EntitySelection.h>
#include <Model/ModelEditor/MaterialProvider.h>
#include <ViewModel/ModelEditor/MaterialListModel.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelEditor/SceneTreeModel.h>
#include <ViewModel/ModelEditor/ViewportModel.h>

#include <utility>

ModelEditorModel::ModelEditorModel(ArrSessionManager* sessionManager, QObject* parent)
    : QObject(parent)
    , m_sessionManager(sessionManager)
    , m_selectedMaterial(nullptr)
{
    m_sceneTreeModel = new SceneTreeModel(sessionManager, this);
    m_materialListModel = new MaterialFilteredListModel(this);
    auto* materialListModel = new MaterialListModel(sessionManager, m_materialListModel);
    m_materialListModel->setSourceModel(materialListModel);

    m_materialListSelectionModel = new QItemSelectionModel(m_materialListModel, this);
    m_editingMaterial = new MaterialProvider(&m_selectedMaterial, sessionManager, this);

    // We link the selected item in the material list to the editing material in the property editor
    QObject::connect(m_materialListSelectionModel, &QItemSelectionModel::selectionChanged, this,
                     [this](const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
                         QModelIndexList selectedIdx = m_materialListSelectionModel->selection().indexes();
                         RR::ApiHandle<RR::Material> material = {};

                         if (selectedIdx.empty())
                         {
                             // when the material list is filtered to be only one material (if you select an entity with just one material)
                             // the editing material is set to that one
                             if (m_materialListModel->isFiltered() && m_materialListModel->getFilteredMaterialSet().size() == 1)
                             {
                                 material = *m_materialListModel->getFilteredMaterialSet().begin();
                             }
                             else
                             {
                                 material = {};
                             }
                         }
                         else
                         {
                             material = selectedIdx.front().data(MaterialListModel::OBJECT_MATERIAL_ROLE).value<RR::ApiHandle<RR::Material>>();
                         }
                         m_selectedMaterial.set(material);
                     });

    QObject::connect(m_materialListModel, &MaterialFilteredListModel::filterChanged, this, [this]() {
        // when the filter is changed, we check if there is only one material in the material list and we select that one
        if (m_materialListModel->rowCount() == 1)
        {
            QModelIndex toSelect = m_materialListModel->index(0, 0);
            auto flags = QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Current;
            m_materialListSelectionModel->select(toSelect, flags);
        }
        else
        {
            // if there is more than one material and the editing material is not one of them, then deselect it
            if (m_materialListModel->isFiltered() && m_selectedMaterial.get() && !m_materialListModel->getFilteredMaterialSet().contains(m_selectedMaterial.get()))
            {
                m_materialListSelectionModel->clearSelection();
            }
        }
    });

    m_treeSelectionModel = new QItemSelectionModel(m_sceneTreeModel, this);

    m_entitySelection = new EntitySelection(this);

    getViewportModel()->setSelectionModel(m_entitySelection);

    //binding between m_treeSelectionModel and m_entitySelection
    connect(m_treeSelectionModel, &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& deselected) {
                if (!m_updatingSelection)
                {
                    m_updatingSelection = true;
                    onSelectionChanged(selected, deselected);
                    m_updatingSelection = false;
                }
            });

    connect(m_entitySelection, &EntitySelection::selectionChanged, this,
            [this](const QList<RR::ApiHandle<RR::Entity>>& added, const QList<RR::ApiHandle<RR::Entity>>& removed) {
                if (!m_updatingSelection)
                {
                    m_updatingSelection = true;
                    onEntitySelectionChanged(added, removed);
                    m_updatingSelection = false;
                }

                // filter on the materials
                QList<RR::ApiHandle<RR::Entity>> selectedEntities;
                for (const auto& entity : *m_entitySelection)
                {
                    selectedEntities.push_back(entity);
                }
                m_materialListModel->filterBasedOnEntities(selectedEntities);
            });


    connect(m_sessionManager, &ArrSessionManager::rootIdChanged, this, [this]() {
        Q_EMIT loadedModelChanged();
    });

    connect(m_sessionManager, &ArrSessionManager::onEnabledChanged, this, [this]() {
        Q_EMIT onEnabledChanged();
    });
}

QAbstractItemModel* ModelEditorModel::getSceneTreeModel() const
{
    return m_sceneTreeModel;
}

QItemSelectionModel* ModelEditorModel::getSceneTreeSelection() const
{
    return m_treeSelectionModel;
}

QAbstractItemModel* ModelEditorModel::getMaterialListModel() const
{
    return m_materialListModel;
}

QItemSelectionModel* ModelEditorModel::getMaterialListSelectionModel() const
{
    return m_materialListSelectionModel;
}

MaterialProvider* ModelEditorModel::getEditingMaterial() const
{
    return m_editingMaterial;
}

ViewportModel* ModelEditorModel::getViewportModel() const
{
    return m_sessionManager->getViewportModel();
}


void ModelEditorModel::onSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
    auto indexes = m_treeSelectionModel->selection().indexes();

    QList<RR::ApiHandle<RR::Entity>> entities;
    entities.reserve(indexes.size());

    for (const QModelIndex& index : indexes)
    {
        entities.push_back(m_sceneTreeModel->getEntityFromIndex(index));
    }
    m_entitySelection->setSelection(entities);
}

void ModelEditorModel::onEntitySelectionChanged(const QList<RR::ApiHandle<RR::Entity>>& added, const QList<RR::ApiHandle<RR::Entity>>& removed)
{
    // potentially slow
    {
        QItemSelection addedSelection;
        for (const RR::ApiHandle<RR::Entity>& entity : added)
        {
            QModelIndex index = m_sceneTreeModel->getIndexFromEntity(entity);
            addedSelection.select(index, index);
        }
        m_treeSelectionModel->select(addedSelection, QItemSelectionModel::SelectionFlag::Select);
    }

    {
        QItemSelection removedSelection;
        for (const RR::ApiHandle<RR::Entity>& entity : removed)
        {
            QModelIndex index = m_sceneTreeModel->getIndexFromEntity(entity);
            removedSelection.select(index, index);
        }
        m_treeSelectionModel->select(removedSelection, QItemSelectionModel::SelectionFlag::Deselect);
    }
}

void ModelEditorModel::unloadModel()
{
    m_sessionManager->unloadModel();
}

QString ModelEditorModel::getLoadedModeName() const
{
    return m_sessionManager->getModelName();
}

bool ModelEditorModel::isEnabled() const
{
    return m_sessionManager->isEnabled();
}
