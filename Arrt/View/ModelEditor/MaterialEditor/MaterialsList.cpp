#include <QHBoxLayout>
#include <QListView>
#include <View/ModelEditor/MaterialEditor/MaterialsList.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>

MaterialListView::MaterialListView(ModelEditorModel* model, QWidget* parent /* = nullptr */)
    : QWidget(parent)
{
    setAccessibleName(tr("Material List"));
    setContentsMargins(0, 0, 0, 0);
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    auto* listView = new QListView(this);
    listView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    listView->setModel(model->getMaterialListModel());
    listView->setSelectionModel(model->getMaterialListSelectionModel());

    l->addWidget(listView);
}
