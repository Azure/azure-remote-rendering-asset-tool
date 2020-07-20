#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/SimpleMessageBox.h>


SimpleMessageBox::SimpleMessageBox(const QString& title, QWidget* parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(title);

    const int unit = fontMetrics().height();
    auto* l = new QVBoxLayout(this);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(unit * 2, 0, unit * 2, unit);
    l->addLayout(m_contentLayout);

    auto* okButton = new FlatButton(tr("OK"), this);
    okButton->setMinimumWidth(unit * 20);
    connect(okButton, &FlatButton::clicked, this, [this]() { close(); });

    okButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    l->addWidget(okButton);
}
