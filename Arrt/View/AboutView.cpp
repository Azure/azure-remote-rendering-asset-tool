#include <QLabel>
#include <QVBoxLayout>
#include <View/AboutView.h>
#include <ViewModel/AboutModel.h>
#include <Widgets/FlatButton.h>

AboutView::AboutView(AboutModel* model, QWidget* parent)
    : SimpleMessageBox(model->getTitle(), parent)
    , m_model(model)
{
    auto* h = new QHBoxLayout();
    h->setContentsMargins(0, 0, 0, 0);
    {
        auto* iconLayout = new QVBoxLayout();
        iconLayout->setContentsMargins(0, 0, 0, 0);
        QPixmap appLogo = m_model->getIcon().pixmap(fontMetrics().height() * 4);
        QLabel* appLogoLabel = new QLabel(this);
        appLogoLabel->setPixmap(appLogo);
        iconLayout->addWidget(appLogoLabel);
        iconLayout->addStretch(1);
        h->addLayout(iconLayout);
    }
    {
        auto* textLayout = new QVBoxLayout();
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->addWidget(new QLabel(m_model->getAppName()));
        textLayout->addWidget(new QLabel(m_model->getCopyrightNotice()));
        textLayout->addWidget(new QLabel(m_model->getVersion()));
        auto* checkReleasesButton = new FlatButton(tr("Check latest releases"));
        connect(checkReleasesButton, &FlatButton::clicked, this, [this]() { m_model->checkLatestReleases(); });
        textLayout->addWidget(checkReleasesButton);
        textLayout->addStretch(1);

        h->addLayout(textLayout);
    }
    getContentLayout()->addLayout(h);
}
