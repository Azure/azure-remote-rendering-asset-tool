#include <QApplication>
#include <QLabel>
#include <QPaintEvent>
#include <QSplitter>
#include <QStylePainter>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <View/ModelEditor/MaterialEditor/MaterialEditorView.h>
#include <View/ModelEditor/MaterialEditor/MaterialsList.h>
#include <View/ModelEditor/ModelEditorView.h>
#include <View/ModelEditor/Scene/ScenePanelView.h>
#include <View/ModelEditor/Stats/StatsPageView.h>
#include <View/ModelEditor/Viewport/ViewportView.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelEditor/Viewport/ViewportModel.h>
#include <ViewUtils/DpiUtils.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FocusableContainer.h>

// the viewport widget has to be wrapped by a non native widget with WA_DontCreateNativeAncestors set,
// to make sure that the parent widgets won't be turned into native widgets. This is because this might break
// the accessibility for the non native widgets (the buttons in the main navigator stop handling the focus
// properly for the narrator)
// Remove this as soon as https://bugreports.qt.io/browse/QTBUG-81862 is fixed and released in 5.15.1 (scheduled in August)

class ContainerForViewport : public QWidget
{
public:
    ContainerForViewport(QWidget* parentWidget)
        : QWidget(parentWidget)
    {
        setContentsMargins(0, 0, 0, 0);

        // temporarily disabling it, because it causes a lot of problems with the viewport (shifted or missing rendering, z-order problems with the session panel)
        //setAttribute(Qt::WA_DontCreateNativeAncestors);
        setAttribute(Qt::WA_LayoutOnEntireRect);
        setMinimumWidth(256);
    }

    void setViewport(QWidget* viewport)
    {
        m_viewport = viewport;
        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(viewport);
        m_viewport->show();
    }
    virtual void resizeEvent(QResizeEvent* event) override
    {
        m_viewport->hide();
        QMetaObject::invokeMethod(QApplication::instance(), [this]() {m_viewport->hide(); m_viewport->show(); });
        QWidget::resizeEvent(event);
    }

private:
    QWidget* m_viewport = nullptr;
};


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
        }
        else
        {
            if (m_uncollapsedSize == 0)
            {
                QWidget* w = splitter()->widget(widgetIdx);
                const QSize s = w->minimumSizeHint();
                m_uncollapsedSize = orientation() == Qt::Horizontal ? s.width() : s.height();
            }
            sizes[widgetIdx] = m_uncollapsedSize;
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

class CustomSplitter : public QSplitter
{
public:
    CustomSplitter(Qt::Orientation orientation, QWidget* parent)
        : QSplitter(orientation, parent)
    {
        connect(this, &QSplitter::splitterMoved, this, [this]() {
            updateCollapsedWidgets();
        });
    }

    void setCollapsedLabelForWidget(int widgetIndex, QString panelName, QIcon icon)
    {
        if (widgetIndex == 0)
        {
            CustomHandle* h = static_cast<CustomHandle*>(handle(1));
            h->setCollapsedLabel(CustomHandle::WidgetLocation::BeforeHandle, panelName, icon);
        }
        else if (widgetIndex == count() - 1)
        {
            CustomHandle* h = static_cast<CustomHandle*>(handle(widgetIndex));
            h->setCollapsedLabel(CustomHandle::WidgetLocation::AfterHandle, panelName, icon);
        }
        updateCollapsedWidgets();
    }

    void setCollapsed(int widgetIndex, bool collapsed)
    {
        if (widgetIndex == 0)
        {
            getHandle(widgetIndex + 1)->setCollapsed(collapsed);
        }
        else if (widgetIndex == count() - 1)
        {
            getHandle(widgetIndex)->setCollapsed(collapsed);
        }
    }

    CustomHandle* getHandle(int index)
    {
        return static_cast<CustomHandle*>(handle(index));
    }

protected:
    virtual QSplitterHandle* createHandle()
    {
        return new CustomHandle(this->orientation(), this);
    }

    virtual void showEvent(QShowEvent* event)
    {
        QSplitter::showEvent(event);
        m_showing = true;
        updateCollapsedWidgets();
    }

private:
    void updateCollapsedWidgets()
    {
        if (m_showing)
        {
            for (int i = 0; i < count(); ++i)
            {
                static_cast<CustomHandle*>(handle(i))->checkCollapsed();
            }
        }
    }

    bool m_showing = false;
};


ModelEditorView::ModelEditorView(ModelEditorModel* modelEditorModel)
    : m_model(modelEditorModel)
{
    auto* l = new QVBoxLayout(this);

    QWidget* toolBar;
    {
        toolBar = new QWidget(this);
        auto* toolbarLayout = new QHBoxLayout(toolBar);

        m_currentlyLoadedModel = new QLabel();
        toolbarLayout->addWidget(m_currentlyLoadedModel);

        auto* unloadButton = new FlatButton(tr("Unload model"), toolBar);
        unloadButton->setToolTip(tr("Unload 3D model"), tr("Unload and go back to the panel for selecting another 3D model"));
        QObject::connect(unloadButton, &FlatButton::clicked, this, [this]() {
            m_model->unloadModel();
        });
        toolbarLayout->addWidget(unloadButton);

        toolbarLayout->addStretch(1);
    }

    l->addWidget(toolBar, 0);

    CustomSplitter* splitter;
    {
        splitter = new CustomSplitter(Qt::Horizontal, this);

        {
            auto scenePanel = new ScenePanelView(modelEditorModel, splitter);
            scenePanel->setMinimumWidth(DpiUtils::size(300));
            splitter->addWidget(new FocusableContainer(scenePanel, splitter));
        }

        {
            auto* viewportSplitter = new CustomSplitter(Qt::Vertical, splitter);

            FocusableContainer* viewportContainer;
            {
                viewportContainer = new FocusableContainer({}, viewportSplitter);
                auto* container = new ContainerForViewport(viewportContainer);
                auto* viewportView = new ViewportView(modelEditorModel->getViewportModel(), container);
                container->setViewport(viewportView);

                auto* viewportLayout = new QHBoxLayout(viewportContainer);
                viewportLayout->setContentsMargins(0, 0, 0, 0);
                viewportLayout->addWidget(container);
            }

            StatsPageView* statsPanel = new StatsPageView(m_model->getStatsPageModel(), viewportSplitter);
            statsPanel->setMinimumHeight(DpiUtils::size(300));

            viewportSplitter->addWidget(viewportContainer);
            viewportSplitter->addWidget(statsPanel);

            viewportSplitter->setCollapsible(0, false);
            viewportSplitter->setCollapsible(1, true);
            viewportSplitter->setStretchFactor(0, 1);
            viewportSplitter->setStretchFactor(1, 0);
            viewportSplitter->setSizes({(int)DpiUtils::size(800), (int)DpiUtils::size(400)});

            viewportSplitter->setCollapsedLabelForWidget(1, tr("Statistics"), ArrtStyle::s_statsIcon);
            viewportSplitter->setCollapsed(1, true);

            splitter->addWidget(viewportSplitter);
        }

        {
            auto* materialSplitter = new QSplitter(Qt::Vertical, splitter);
            auto* materialListView = new MaterialListView(modelEditorModel, materialSplitter);
            auto* materialEditorView = new MaterialEditorView(modelEditorModel->getEditingMaterial(), materialSplitter);

            materialSplitter->addWidget(new FocusableContainer(materialListView, materialSplitter));
            materialSplitter->addWidget(materialEditorView);
            materialSplitter->setChildrenCollapsible(false);
            materialSplitter->setMinimumWidth(DpiUtils::size(150));
            materialSplitter->setStretchFactor(0, 0);
            materialSplitter->setStretchFactor(1, 1);
            materialSplitter->setSizes({(int)DpiUtils::size(150), (int)DpiUtils::size(500)});
            materialSplitter->setMinimumWidth(DpiUtils::size(300));

            splitter->addWidget(materialSplitter);
        }

        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        splitter->setStretchFactor(2, 0);

        splitter->setCollapsible(1, false);

        splitter->setCollapsedLabelForWidget(0, tr("Scene entities"), ArrtStyle::s_sceneIcon);
        splitter->setCollapsedLabelForWidget(2, tr("Materials"), ArrtStyle::s_materialsIcon);
        splitter->setSizes({(int)DpiUtils::size(300), (int)DpiUtils::size(800), (int)DpiUtils::size(300)});
        splitter->setCollapsed(0, true);
    }

    l->addWidget(splitter, 1);

    auto onEnabledChanged = [this]() {
        setEnabled(m_model->isEnabled());
    };
    onEnabledChanged();
    QObject::connect(m_model, &ModelEditorModel::onEnabledChanged, this, onEnabledChanged);

    QObject::connect(m_model, &ModelEditorModel::loadedModelChanged, this, [this]() { updateUi(); });
    updateUi();
}

void ModelEditorView::updateUi()
{
    m_currentlyLoadedModel->setText("<h3>" + m_model->getLoadedModeName() + "</h3>");
}

ModelEditorView::~ModelEditorView()
{
}
