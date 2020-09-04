#include <QHBoxLayout>
#include <QTreeView>
#include <View/ModelEditor/Scene/ScenePanelView.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>

ScenePanelView::ScenePanelView(ModelEditorModel* model, QWidget* parent /* = nullptr */)
    : QWidget(parent)
    , m_model(model)
{
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    auto* treeView = new QTreeView(this);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setHeaderHidden(true);
    treeView->setModel(model->getSceneTreeModel());
    treeView->setSelectionModel(model->getSceneTreeSelection());

    connect(model->getSceneTreeSelection(), &QItemSelectionModel::selectionChanged, this,
            [treeView](const QItemSelection& selected, const QItemSelection& /*deselected*/) {
                // Make sure when an item is selected, it is visible.
                // This will expand the tree if the item is in a collapsed branch
                // and scroll the view, if needed
                QModelIndexList indices = selected.indexes();
                if (!indices.empty())
                {
                    treeView->scrollTo(indices[0]);
                }
            });
    connect(treeView, &QTreeView::doubleClicked, this,
            [this](const QModelIndex& index) {
                m_model->focusEntity(index);
            });

    l->addWidget(treeView);
}
