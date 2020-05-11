#pragma once
#include <Model/Configuration.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QItemSelectionModel>
#include <QObject>
#include <Utils/Value.h>

class ArrSessionManager;
class SceneTreeModel;
class MaterialFilteredListModel;
class MaterialProvider;
class ViewportModel;
class EntitySelection;

// ViewModel for ModelEditorView, exposing all of the models used by the Material editor and Scene tree

class ModelEditorModel : public QObject
{
    Q_OBJECT

public:
    ModelEditorModel(ArrSessionManager* sessionManager, QObject* parent);

    QAbstractItemModel* getSceneTreeModel() const;
    QItemSelectionModel* getSceneTreeSelection() const;

    QAbstractItemModel* getMaterialListModel() const;
    QItemSelectionModel* getMaterialListSelectionModel() const;

    MaterialProvider* getEditingMaterial() const;
    ViewportModel* getViewportModel() const;

    void unloadModel();

    QString getLoadedModeName() const;

    bool isEnabled() const;

Q_SIGNALS:
    void onEnabledChanged();
    void loadedModelChanged();

private:
    ArrSessionManager* const m_sessionManager;
    SceneTreeModel* m_sceneTreeModel = nullptr;
    MaterialFilteredListModel* m_materialListModel = nullptr;
    QItemSelectionModel* m_materialListSelectionModel = nullptr;
    MaterialProvider* m_editingMaterial = nullptr;
    Value<std::shared_ptr<RR::Material>> m_selectedMaterial;

    QItemSelectionModel* m_treeSelectionModel = nullptr;

    EntitySelection* m_entitySelection = nullptr;

    // used to inhibit selection operation callbacks
    bool m_updatingSelection = false;

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onEntitySelectionChanged(const QList<std::shared_ptr<RR::Entity>>& added, const QList<std::shared_ptr<RR::Entity>>& removed);
};
