#include <QBoxLayout>
#include <QIcon>
#include <QResizeEvent>
#include <QStylePainter>
#include <View/ArrtStyle.h>
#include <ViewUtils/DpiUtils.h>
#include <Widgets/CustomSplitter.h>
#include <Widgets/FlatButton.h>

class CollapsedPanelLabel : public QWidget
{
public:
    CollapsedPanelLabel(Qt::Orientation orientation, QString name, const QIcon icon, QWidget* parent = {})
        : QWidget(parent)
        , m_orientation(orientation)
        , m_name(name)
        , m_icon(icon)
    {
        setContentsMargins(QMargins() + (int)DpiUtils::size(2));
        setFont(ArrtStyle::s_splitterHandleFont);
    }

    virtual void paintEvent(QPaintEvent* /*event*/) override
    {
        QStylePainter p(this);
        QRect r = contentsRect();

        if (m_orientation == Qt::Horizontal)
        {
            p.rotate(90);
            p.translate(0, -width());
            r.setSize(r.size().transposed());
        }
        QRect iconRect = r;
        iconRect.setWidth(iconRect.height());
        p.drawPixmap(iconRect, m_icon.pixmap(iconRect.size()));
        QRect textRect = r;
        textRect.setLeft(iconRect.right() + DpiUtils::size(s_spacing));
        p.setPen(palette().text().color());
        p.drawText(textRect, m_name);
    }

    virtual QSize minimumSizeHint() const override
    {
        QSize s;
        const int h = fontMetrics().height();
        const int w = fontMetrics().horizontalAdvance(m_name) + h + DpiUtils::size(s_spacing) + h; //adds a bit more space to the right
        const int m = contentsMargins().left() + contentsMargins().right();

        return m_orientation == Qt::Vertical ? QSize(w + m, h + m) : QSize(h + m, w + m);
    }

private:
    Qt::Orientation m_orientation;
    QString m_name;
    QIcon m_icon;
    static const int s_spacing = 4;
};

class CustomHandle : public QSplitterHandle
{
public:
    enum class WidgetLocation
    {
        NotSet,
        BeforeHandle,
        AfterHandle
    };

    CustomHandle(Qt::Orientation orientation, QSplitter* parent)
        : QSplitterHandle(orientation, parent)
    {
        m_container = new QWidget(this);
        m_container->setContentsMargins({});
        if (orientation == Qt::Horizontal)
        {
            m_containerLayout = new QVBoxLayout(m_container);
        }
        else
        {
            m_containerLayout = new QHBoxLayout(m_container);
        }
        m_containerLayout->setContentsMargins({});
        m_containerLayout->addStretch(1);

        m_collapsed = false;

        m_collapseButton = new FlatButton("", this);
        QSize buttonSize(DpiUtils::size(60), DpiUtils::size(15));
        m_collapseButton->setFixedSize(orientation == Qt::Vertical ? buttonSize : buttonSize.transposed());

        QBoxLayout* l;
        if (orientation == Qt::Horizontal)
        {
            l = new QVBoxLayout(this);
        }
        else
        {
            l = new QHBoxLayout(this);
        }

        l->setContentsMargins({});
        l->addWidget(m_container, 1);
        l->addWidget(m_collapseButton);
        l->addStretch(1);

        updateButtons();

        connect(m_collapseButton, &FlatButton::clicked, this, [this]() { setCollapsed(!m_collapsed); });
    }

    void setCollapsedLabel(WidgetLocation collapsibleWidgetLocation, QString name, QIcon icon)
    {
        m_name = name;
        delete m_label;
        m_label = new CollapsedPanelLabel(orientation(), name, icon, m_container);
        m_collapsibleWidgetLocation = collapsibleWidgetLocation;
        m_containerLayout->insertWidget(0, m_label);
        m_label->setVisible(m_collapsed);
        updateButtons();
    }

    void setCollapsed(bool collapsed)
    {
        if (m_collapsed == collapsed)
        {
            return;
        }
        const int widgetIdx = getCollapsibledWidgetIndex();
        if (widgetIdx < 0)
        {
            return;
        }

        auto sizes = splitter()->sizes();
        if (collapsed)
        {
            m_uncollapsedSize = sizes[widgetIdx];
            sizes[widgetIdx] = 0;
            splitter()->widget(widgetIdx)->setEnabled(false);
        }
        else
        {
            m_container->setVisible(true);

            if (m_uncollapsedSize == 0)
            {
                QWidget* w = splitter()->widget(widgetIdx);
                const QSize s = w->minimumSizeHint();
                m_uncollapsedSize = orientation() == Qt::Horizontal ? s.width() : s.height();
            }
            sizes[widgetIdx] = m_uncollapsedSize;
            splitter()->widget(widgetIdx)->setEnabled(true);
        }
        splitter()->setSizes(sizes);

        checkCollapsed();
    }


    void checkCollapsed()
    {
        if (m_collapsibleWidgetLocation == WidgetLocation::NotSet)
        {
            return;
        }
        const int widgetIdx = getCollapsibledWidgetIndex();
        if (widgetIdx < 0)
        {
            return;
        }
        const auto sizes = splitter()->sizes();
        const bool isCollapsed = sizes[widgetIdx] == 0;
        if (m_collapsed != isCollapsed)
        {
            m_collapsed = isCollapsed;
            if (m_label)
            {
                m_label->setVisible(isCollapsed);
            }
            updateButtons();
        }
    }

    WidgetLocation getCollapsibleWidgetLocation()
    {
        return m_collapsibleWidgetLocation;
    }

    virtual QSize sizeHint() const override
    {
        QSize def = QSplitterHandle::sizeHint();
        QSize minContentSize = static_cast<const QBoxLayout*>(layout())->minimumSize();

        if (orientation() == Qt::Horizontal)
        {
            def.setWidth(minContentSize.width());
        }
        else
        {
            def.setHeight(minContentSize.height());
        }
        return def;
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
    }

private:
    QWidget* m_container;
    QBoxLayout* m_containerLayout;
    QString m_name;
    CollapsedPanelLabel* m_label = {};

    int m_uncollapsedSize = 0;

    WidgetLocation m_collapsibleWidgetLocation = WidgetLocation::NotSet;
    FlatButton* m_collapseButton = {};
    bool m_collapsed = false;
    mutable int m_thisHandleIndex = -1;

    int getThisHandleIndex() const
    {
        if (m_thisHandleIndex < 0)
        {
            // find the index of this handle
            for (m_thisHandleIndex = splitter()->count() - 1; m_thisHandleIndex >= 0; --m_thisHandleIndex)
            {
                if (splitter()->handle(m_thisHandleIndex) == this)
                {
                    break;
                }
            }
        }
        return m_thisHandleIndex;
    }

    int getCollapsibledWidgetIndex() const
    {
        if (m_collapsibleWidgetLocation == WidgetLocation::NotSet)
        {
            return -1;
        }
        const int idx = getThisHandleIndex();
        if (idx < 0)
        {
            return -1;
        }
        const int widgetIdx = (m_collapsibleWidgetLocation == WidgetLocation::AfterHandle) ? idx : (idx - 1);
        if (idx >= splitter()->count())
        {
            return -1;
        }
        return widgetIdx;
    }

    void updateButtons()
    {
        if (m_collapsibleWidgetLocation == WidgetLocation::NotSet)
        {
            m_collapseButton->setVisible(false);
        }
        else
        {
            m_collapseButton->setVisible(true);
            if (orientation() == Qt::Vertical)
            {
                bool up = m_collapsibleWidgetLocation == WidgetLocation::BeforeHandle;
                if (m_collapsed)
                {
                    up = !up;
                }
                m_collapseButton->setIcon(up ? ArrtStyle::s_arrowUpIcon : ArrtStyle::s_arrowDownIcon);
            }
            else
            {
                bool left = m_collapsibleWidgetLocation == WidgetLocation::BeforeHandle;
                if (m_collapsed)
                {
                    left = !left;
                }
                m_collapseButton->setIcon(left ? ArrtStyle::s_arrowLeftIcon : ArrtStyle::s_arrowRightIcon);
            }

            m_collapseButton->setText((m_collapsed ? tr("Show %1 panel") : tr("Hide %1 panel")).arg(m_name));
        }
    }
};


CustomSplitter::CustomSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
{
    connect(this, &QSplitter::splitterMoved, this, [this]() {
        updateCollapsedWidgets();
    });
}

bool CustomSplitter::makeWidgetCollapsible(WidgetPosition widgetPosition, QString panelName, QIcon icon)
{
    if (count() < 2)
    {
        return false;
    }
    int index;
    if (widgetPosition == FIRST)
    {
        index = 0;
        CustomHandle* h = static_cast<CustomHandle*>(handle(index + 1));
        h->setCollapsedLabel(CustomHandle::WidgetLocation::BeforeHandle, panelName, icon);
    }
    else if (widgetPosition == LAST)
    {
        index = count() - 1;
        CustomHandle* h = static_cast<CustomHandle*>(handle(index));
        h->setCollapsedLabel(CustomHandle::WidgetLocation::AfterHandle, panelName, icon);
    }
    else
    {
        return false;
    }
    updateCollapsedWidgets();
    return true;
}

void CustomSplitter::setCollapsed(WidgetPosition widgetPosition, bool collapsed)
{
    if (widgetPosition == WidgetPosition::FIRST)
    {
        getHandle(1)->setCollapsed(collapsed);
    }
    else if (widgetPosition == WidgetPosition::LAST)
    {
        getHandle(count() - 1)->setCollapsed(collapsed);
    }
}

CustomHandle* CustomSplitter::getHandle(int index)
{
    return static_cast<CustomHandle*>(handle(index));
}

QSplitterHandle* CustomSplitter::createHandle()
{
    return new CustomHandle(this->orientation(), this);
}

void CustomSplitter::showEvent(QShowEvent* event)
{
    QSplitter::showEvent(event);
    m_showing = true;
    updateCollapsedWidgets();
    QSplitter::showEvent(event);
}

void CustomSplitter::updateCollapsedWidgets()
{
    if (m_showing)
    {
        for (int i = 0; i < count(); ++i)
        {
            static_cast<CustomHandle*>(handle(i))->checkCollapsed();
        }
    }
}
