#include <QLabel>
#include <QVBoxLayout>
#include <View/NewVersionView.h>
#include <ViewModel/NewVersionModel.h>
#include <Widgets/FlatButton.h>

NewVersionView::NewVersionView(NewVersionModel* model, QWidget* parent)
    : SimpleMessageBox(model->getTitle(), parent)
    , m_model(model)
{
    QString text = tr("<h3>New version available!</h3>Current version: %1<br>Latest version: %2").arg(m_model->getCurrentVersion()).arg(m_model->getLatestVersion());
    getContentLayout()->addWidget(new QLabel(text));

    {
        QString directDownloadText = tr("Direct download %1").arg(m_model->getLatestVersion());

        FlatButton* directDownload = new FlatButton(directDownloadText, this);
        connect(directDownload, &FlatButton::clicked, this, [this]() { m_model->downloadLatestRelease(); });
        getContentLayout()->addWidget(directDownload);
    }
    {
        FlatButton* goToRelease = new FlatButton(tr("Go to releases"), this);
        connect(goToRelease, &FlatButton::clicked, this, [this]() { m_model->goToLatestReleases(); });
        getContentLayout()->addWidget(goToRelease);
    }
}
