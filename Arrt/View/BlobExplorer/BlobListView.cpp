#include <Model/IncludesAzureStorage.h>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPen>
#include <QStyledItemDelegate>
#include <ViewUtils/DpiUtils.h>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobListView.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);
Q_DECLARE_METATYPE(BlobsListModel::LoadingStatus);
Q_DECLARE_METATYPE(BlobsListModel::EntryType);

// delegate class for displaying 3d models coming from a BlobTreeModel

class ModelDelegate : public QStyledItemDelegate
{
public:
    ModelDelegate(QObject* parent)
        : QStyledItemDelegate(parent)
    {
    }

    void setDirectoryMode(bool directoryMode)
    {
        m_directoryMode = directoryMode;
    }

    void drawText(QPainter* painter, const QWidget* w, const QString& text, const QFont& font, const QColor& color, const QRect& rect) const
    {
        QFontMetrics fm(font, w);

        painter->setFont(font);
        painter->setPen(QPen(color));
        QString elidedText = fm.elidedText(text, Qt::TextElideMode::ElideRight, rect.width(), Qt::TextSingleLine | Qt::TextDontClip);
        QTextOption to(Qt::AlignVCenter | Qt::AlignLeft);
        to.setWrapMode(QTextOption::NoWrap);
        painter->drawText(rect, elidedText, to);
    }

    virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        option->text.clear();
    }

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        const bool selected = option.state & QStyle::State_Selected;

        QRect rect = option.rect;

        painter->setPen(ArrtStyle::s_listSeparatorColor);
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());

        QStyledItemDelegate::paint(painter, option, index);

        auto loadingStatus = index.data(BlobsListModel::LOADING_STATUS_ROLE).value<BlobsListModel::LoadingStatus>();

        const int margin = DpiUtils::size(s_margin);
        QRect textRect(rect.adjusted(margin, margin, -margin, -margin));

        if (loadingStatus != BlobsListModel::LoadingStatus::NOT_LOADED)
        {
            QRect loadingUiRect(textRect);

            switch (loadingStatus)
            {
                // compilation warning fix
                case BlobsListModel::LoadingStatus::NOT_LOADED:
                    break;
                case BlobsListModel::LoadingStatus::FAILED:
                    loadingUiRect.setLeft(loadingUiRect.right() - DpiUtils::size(50));
                    drawText(painter, option.widget, BlobsListModel::statusToString(loadingStatus), ArrtStyle::s_blobStatusFont, ArrtStyle::s_failureColor, loadingUiRect);
                    break;
                case BlobsListModel::LoadingStatus::LOADED:
                    loadingUiRect.setLeft(loadingUiRect.right() - DpiUtils::size(50));
                    drawText(painter, option.widget, BlobsListModel::statusToString(loadingStatus), ArrtStyle::s_blobStatusFont, ArrtStyle::s_successColor, loadingUiRect);
                    break;
                case BlobsListModel::LoadingStatus::LOADING:
                    loadingUiRect.setLeft(loadingUiRect.right() - DpiUtils::size(120));
                    drawText(painter, option.widget, BlobsListModel::statusToString(loadingStatus), ArrtStyle::s_blobStatusFont, option.palette.color(selected ? QPalette::HighlightedText : QPalette::Midlight), loadingUiRect);
                    float loadingProgress = index.data(BlobsListModel::LOADING_PROGRESS).value<float>();
                    if (loadingProgress < 0)
                    {
                        loadingProgress = 0;
                    }
                    QPoint p1 = loadingUiRect.center();
                    QPoint p2 = p1;
                    p2.setX(loadingUiRect.right());
                    QPoint pProgress = p1 + (p2 - p1) * loadingProgress;

                    painter->setPen(QPen(ArrtStyle::s_progressColor, 2));
                    painter->drawLine(p1, pProgress);
                    painter->setPen(QPen(ArrtStyle::s_progressBackgroundColor, 2));
                    painter->drawLine(pProgress, p2);
                    break;
            }
            textRect.setRight(loadingUiRect.left() - DpiUtils::size(5));
        }

        QString name = index.data().toString();
        QString path = index.data(BlobsListModel::PATH_ROLE).toString();
        QIcon icon;
        switch (index.data(BlobsListModel::ENTRY_TYPE_ROLE).value<BlobsListModel::EntryType>())
        {
            case BlobsListModel::EntryType::DirectoryUp:
                icon = ArrtStyle::s_parentdirIcon;
                break;
            case BlobsListModel::EntryType::Directory:
                icon = ArrtStyle::s_directoryIcon;
                break;
            case BlobsListModel::EntryType::Model:
                icon = ArrtStyle::s_modelIcon;
                break;
            case BlobsListModel::EntryType::ConfigFile:
                break;
            case BlobsListModel::EntryType::Texture:
                break;
            case BlobsListModel::EntryType::Other:
                break;
            case BlobsListModel::EntryType::NoType:
                break;
        }

        // leave space for an icon draw
        QRect iconRect = textRect;
        iconRect.setWidth(DpiUtils::size(40));

        // draw the scaled icon (to be replaced with SVG icons)
        QRect actualIconRect;
        actualIconRect.setSize({(int)DpiUtils::size(20), (int)DpiUtils::size(20)});
        actualIconRect.moveCenter(iconRect.center());
        painter->drawPixmap(actualIconRect, icon.pixmap(actualIconRect.size()));

        // draw the text
        textRect.setLeft(iconRect.right());

        QRect nameRect = textRect;
        if (!m_directoryMode)
        {
            int pathHeight = QFontMetrics(ArrtStyle::s_blobPathFont, option.widget).height() + s_spacer / 2;
            nameRect.adjust(0, 0, 0, -pathHeight);
            QRect pathRect = textRect;
            pathRect.setTop(nameRect.bottom());
            drawText(painter, option.widget, path, ArrtStyle::s_blobPathFont, selected ? option.palette.highlightedText().color() : ArrtStyle::s_underTextColor, pathRect);
        }
        drawText(painter, option.widget, index.data().toString(), ArrtStyle::s_blobNameFont, option.palette.color(selected ? QPalette::HighlightedText : QPalette::WindowText), nameRect);
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
    {
        int height = QFontMetrics(ArrtStyle::s_blobNameFont, option.widget).height() + s_margin * 2;
        if (!m_directoryMode)
        {
            height += QFontMetrics(ArrtStyle::s_blobPathFont, option.widget).height() + s_spacer;
        }
        return QSize(1, height);
    }

private:
    static const int s_spacer = 6;
    static const int s_margin = 2;
    bool m_directoryMode = false;
};

BlobListView::BlobListView(BlobsListModel* model, QWidget* parent)
    : QListView(parent)
    , m_model(model)
{
    setModel(model);

    m_delegate = new ModelDelegate(this);
    setItemDelegate(m_delegate);

    connect(this, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        m_model->doubleClickItem(index);
    });

    QItemSelectionModel* selectionModel = new QItemSelectionModel(model, model);
    setSelectionModel(selectionModel);

    connect(selectionModel, &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex& /*previous*/) {
                m_model->setCurrentItem(current);
            });

    auto onFilterChanged = [this]() {
        m_delegate->setDirectoryMode(m_model->getFilterType() != BlobsListModel::FilterType::JustAllowedExtensions);
        reset();
    };
    connect(m_model, &BlobsListModel::filterTypeChanged, this, onFilterChanged);
    onFilterChanged();
}

void BlobListView::setAcceptFileDrops(bool acceptDrop)
{
    if (m_acceptFileDrop != acceptDrop)
    {
        m_acceptFileDrop = acceptDrop;
        if (m_acceptFileDrop)
        {
            setAcceptDrops(true);
        }
    }
}

void BlobListView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void BlobListView::dragMoveEvent(QDragMoveEvent* e)
{
    e->accept();
}

void BlobListView::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (event->proposedAction() == Qt::CopyAction && mimeData->hasUrls())
    {
        event->acceptProposedAction();

        // accepts all of the urls passed
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // extract the local paths of the files
        for (auto& path : urlList)
        {
            pathList.append(path.toLocalFile());
        }
        // call a function to open the files
        Q_EMIT filesDropped(pathList);
    }
}

void BlobListView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        BlobsListModel::EntryType type;
        QString path;
        QString url;
        m_model->getCurrentItem(type, path, url);

        if (type != BlobsListModel::EntryType::NoType && type != BlobsListModel::EntryType::DirectoryUp)
        {
            QMessageBox mb(this);
            if (type == BlobsListModel::EntryType::Directory)
            {
                mb.setText(tr("Deleting directory %0 and its subdirectories. Proceed?").arg(path));
            }
            else
            {
                mb.setText(tr("Deleting blob %0. Proceed?").arg(path));
            }
            mb.setIcon(QMessageBox::Warning);
            mb.setWindowTitle(tr("Deleting"));
            mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            mb.setDefaultButton(QMessageBox::Ok);
            if (mb.exec() == QMessageBox::Ok)
            {
                m_model->deleteCurrentItem();
            }
        }
    }
}
