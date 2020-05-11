#include <QHBoxLayout>
#include <QListView>
#include <View/ModelEditor/MaterialsList.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>

MaterialListView::MaterialListView(ModelEditorModel* model, QWidget* parent /* = nullptr */)
    : QWidget(parent)
{
    auto* l = new QHBoxLayout(this);
    auto* listView = new QListView(this);
    listView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    listView->setModel(model->getMaterialListModel());
    listView->setSelectionModel(model->getMaterialListSelectionModel());

    l->addWidget(listView);
}
