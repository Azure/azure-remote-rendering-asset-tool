#include <QHBoxLayout>
#include <QTreeView>
#include <View/ModelEditor/ScenePanelView.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>

ScenePanelView::ScenePanelView(ModelEditorModel* model, QWidget* parent /* = nullptr */)
    : QWidget(parent)
{
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    auto* treeView = new QTreeView(this);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setHeaderHidden(true);
    treeView->setModel(model->getSceneTreeModel());
    treeView->setSelectionModel(model->getSceneTreeSelection());

    QObject::connect(model->getSceneTreeSelection(), &QItemSelectionModel::selectionChanged, this,
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
    l->addWidget(treeView);
}
