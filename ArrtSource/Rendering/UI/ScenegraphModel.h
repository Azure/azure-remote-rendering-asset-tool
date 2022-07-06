#pragma once

#include <QAbstractItemModel>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <map>
#include <vector>

class ArrSession;

struct SceneEntry
{
    QString m_name;
    int m_rowIndex = -1;
    bool m_traversedChildren = false;
    SceneEntry* m_parent = nullptr;
    std::vector<SceneEntry> m_children;
    RR::ApiHandle<RR::Entity> m_arrEntity;
};

/// The QAbstractItemModel for the scene-graph tree view.
class ScenegraphModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ScenegraphModel(ArrSession* session);
    ~ScenegraphModel();

    void RefreshModel();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    RR::ApiHandle<RR::Entity> GetEntityHandle(const QModelIndex& index) const;

    QModelIndex MapArrEntityHandleToModelIndex(const RR::ApiHandle<RR::Entity>& handle) const;

private:
    /// Recursively traverses the entire entity hierarchy and stores the necessary information to access each piece
    void FillChildEntries(SceneEntry* entry) const;

    mutable std::vector<SceneEntry> m_modelRoots;

    ArrSession* m_arrSession = nullptr;

    mutable std::map<unsigned long long, QModelIndex> m_ArrHandleToQt;
};