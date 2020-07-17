#include <QLabel>
#include <QVBoxLayout>
#include <View/NewVersionView.h>
#include <ViewModel/NewVersionModel.h>
#include <Widgets/FlatButton.h>

NewVersionView::NewVersionView(NewVersionModel* model, QWidget* parent)
    : SimpleMessageBox(model->getTitle(), parent)
    , m_model(model)
{
    getContentLayout()->addWidget(new QLabel("Test"));
}
