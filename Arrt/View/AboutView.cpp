#include <QLabel>
#include <QVBoxLayout>
#include <View/AboutView.h>
#include <ViewModel/AboutModel.h>
#include <Widgets/FlatButton.h>

AboutView::AboutView(AboutModel* model, QWidget* parent)
    : QDialog(parent)
    , m_model(model)
{
    const int unit = fontMetrics().height();

    auto* l = new QVBoxLayout(this);
    this->setWindowTitle(m_model->getTitle());


    {
        auto* h = new QHBoxLayout();
        h->setContentsMargins(unit * 2, 0, unit * 2, unit);

        h->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        {
            auto* iconLayout = new QVBoxLayout();
            iconLayout->setContentsMargins(0, 0, 0, 0);
            QPixmap appLogo = m_model->getIcon().pixmap(unit * 8);
            QLabel* appLogoLabel = new QLabel(this);
            appLogoLabel->setAlignment(Qt::AlignTop);
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

        l->addLayout(h);
    }

    auto* okButton = new FlatButton(tr("OK"), this);
    okButton->setMinimumWidth(unit * 20);
    connect(okButton, &FlatButton::clicked, this, [this]() { close(); });

    okButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    l->addWidget(okButton);
}
