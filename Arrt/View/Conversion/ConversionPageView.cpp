#include <View/Conversion/ConversionPageView.h>
#include <View/Conversion/ConversionView.h>

#include <QHBoxLayout>
#include <QItemDelegate>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <ViewModel/Conversion/ConversionModel.h>
#include <ViewModel/Conversion/ConversionPageModel.h>
#include <ViewModel/Conversion/CurrentConversionsModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FocusableContainer.h>

Q_DECLARE_METATYPE(Conversion::Status);

namespace
{
    QIcon getIconFromStatus(Conversion::Status status)
    {
        switch (status)
        {
            case Conversion::NOT_STARTED:
            case Conversion::UNKNOWN:
                return {};
            case Conversion::START_REQUESTED:
            case Conversion::STARTING:
            case Conversion::SYNCHRONIZING:
            case Conversion::CONVERTING:
                return ArrtStyle::s_conversion_runningIcon;
            case Conversion::COMPLETED:
                return ArrtStyle::s_conversion_succeededIcon;
            case Conversion::CANCELED:
                return ArrtStyle::s_conversion_canceledIcon;
            case Conversion::SYNCHRONIZATION_FAILED:
            case Conversion::CONVERSION_FAILED:
            case Conversion::FAILED_TO_START:
                return ArrtStyle::s_conversion_failedIcon;
        }
        return {};
    }
} // namespace


class ConversionListDelegate : public QItemDelegate
{
public:
    ConversionListDelegate(CurrentConversionsModel* /*model*/, QObject* parent = {})
        : QItemDelegate(parent)
    {
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
    {
        QFontMetrics fm(ArrtStyle::s_conversionListFont, option.widget);
        return QSize(20, fm.height() + m_spacing * 2);
    }

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        bool selected = option.state & QStyle::State_Selected;

        QString name = index.data(CurrentConversionsModel::NAME).toString();
        int seconds = index.data(CurrentConversionsModel::INGESTION_TIME).toInt();
        QString timeString = tr("%1:%2").arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));

        Conversion::Status status = index.data(CurrentConversionsModel::STATUS).value<Conversion::Status>();

        QColor textColor = option.palette.text().color();

        if (selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
            textColor = option.palette.highlightedText().color();
        }

        QRect r = option.rect.adjusted(m_spacing, m_spacing, -m_spacing, -m_spacing);

        //draw icon
        QRect iconRect = r;
        //make it square, and on the left side
        iconRect.setRight(iconRect.left() + iconRect.height());
        QIcon icon = getIconFromStatus(status);
        icon.paint(painter, iconRect, Qt::AlignCenter);
        r.setLeft(iconRect.right() + m_spacing);

        //draw time
        painter->setPen(textColor.darker());
        QRect timeRect = r;
        timeRect.setLeft(timeRect.right() - QFontMetrics(ArrtStyle::s_conversionTimeFont, option.widget).horizontalAdvance(timeString));
        painter->setFont(ArrtStyle::s_conversionTimeFont);
        painter->drawText(timeRect, Qt::AlignCenter, timeString);
        r.setRight(timeRect.left() - m_spacing);

        //draw name
        painter->setPen(textColor);
        QFontMetrics fm(ArrtStyle::s_conversionListFont, option.widget);
        name = fm.elidedText(name, Qt::TextElideMode::ElideRight, r.width());
        painter->setFont(ArrtStyle::s_conversionListFont);
        painter->drawText(r, Qt::AlignBaseline, name);
    }

private:
    const int m_spacing = 3;
};


ConversionPageView::ConversionPageView(ConversionPageModel* model, QWidget* parent)
    : QSplitter(parent)
    , m_model(model)
{
    {
        auto* listPanel = new QWidget(this);
        auto* conversionList = new QListView();
        conversionList->setItemDelegate(new ConversionListDelegate(m_model->getCurrentConversionsModel(), conversionList));

        auto* listLayout = new QVBoxLayout(listPanel);
        auto* toolbarLayout = new QHBoxLayout();
        FlatButton* addConversion = new FlatButton(tr("New"), listPanel);
        addConversion->setToolTip(tr("New conversion"), tr("Create a new task to convert a 3D model for Remote Rendering"));
        addConversion->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        addConversion->setIcon(ArrtStyle::s_newIcon, true);
        connect(addConversion, &FlatButton::clicked, this, [this]() { m_model->addNewConversion(); });

        m_removeConversionButton = new FlatButton(tr("Delete"), listPanel);
        m_removeConversionButton->setToolTip(tr("Delete conversion"), tr("Removes the selected conversion task from the list. It can only be called on conversions that are not running"));
        m_removeConversionButton->setIcon(ArrtStyle::s_removeIcon, true);
        m_removeConversionButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        connect(m_removeConversionButton, &FlatButton::clicked, this, [this]() { m_model->removeCurrentConversion(); });

        toolbarLayout->addWidget(addConversion);
        toolbarLayout->addWidget(m_removeConversionButton);

        m_description = ArrtStyle::createHeaderLabel({}, tr("Add a new conversion to start converting your 3D model into an arrAsset"));

        listLayout->addWidget(m_description, 0);
        listLayout->addLayout(toolbarLayout, 0);
        listLayout->addWidget(new FocusableContainer(conversionList, listPanel), 1);

        auto updateLabelVisbiilty = [this]() {
            m_description->setVisible(m_model->getCurrentConversionsModel()->rowCount() == 0);
        };
        connect(model->getCurrentConversionsModel(), &QStandardItemModel::rowsInserted, this, updateLabelVisbiilty);
        connect(model->getCurrentConversionsModel(), &QStandardItemModel::rowsRemoved, this, updateLabelVisbiilty);
        connect(model->getCurrentConversionsModel(), &QStandardItemModel::layoutChanged, this, updateLabelVisbiilty);
        connect(model->getCurrentConversionsModel(), &QStandardItemModel::modelReset, this, updateLabelVisbiilty);
        updateLabelVisbiilty();

        CurrentConversionsModel* currentConversionsModel = m_model->getCurrentConversionsModel();
        conversionList->setModel(currentConversionsModel);
        conversionList->setSelectionModel(m_model->getSelectionModel());

        conversionList->setCurrentIndex(conversionList->model()->index(0, 0));

        addWidget(listPanel);
        setStretchFactor(0, 0);
    }

    {
        auto* selectedConversionView = new ConversionView(m_model->getSelectedConversionModel(), this);
        addWidget(selectedConversionView);
        setStretchFactor(1, 1);
    }

    auto onEnabledChanged = [this]() {
        setEnabled(m_model->isEnabled());
    };
    onEnabledChanged();
    QObject::connect(m_model, &ConversionPageModel::onEnabledChanged, onEnabledChanged);

    connect(m_model, &ConversionPageModel::changed, this, [this]() { updateUi(); });
    updateUi();
}

void ConversionPageView::updateUi()
{
    m_removeConversionButton->setEnabled(m_model->canRemoveCurrentConversion());
}
