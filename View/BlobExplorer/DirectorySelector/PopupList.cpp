#include <QApplication>
#include <QKeyEvent>
#include <QPointer>
#include <Utils/FontOverrideModel.h>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/DirectorySelector/PopupList.h>

PopupList::PopupList(QAbstractItemModel* m, QWidget* parent)
    : QListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMouseTracking(true);
    auto* overriddenModel = new FontOverrideListModel(ArrtStyle::s_directoryPopupFont, m, this);
    setModel(overriddenModel);
    m->setParent(overriddenModel);
    setUniformItemSizes(true);

    connect(m, &QAbstractItemModel::rowsInserted, [this]() {
        if (model()->rowCount() == 1)
        {
            selectionModel()->setCurrentIndex(model()->index(0, 0), QItemSelectionModel::Current);
        }
        adjustHeight();
    });
    adjustHeight();

    setSelectionMode(QAbstractItemView::MultiSelection);
    auto* selModel = new QItemSelectionModel(overriddenModel);
    connect(selModel, &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& newSel, const QItemSelection&) {
        QModelIndexList items = newSel.indexes();
        if (!items.empty())
        {
            Q_EMIT selected(items[0].data(Qt::UserRole).toString());
            finished();
        }
    });
    setSelectionModel(selModel);

    connect(this, &QListView::entered, [this](const QModelIndex& index) {
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current);
    });
    selectionModel()->setCurrentIndex(overriddenModel->index(0, 0), QItemSelectionModel::Current);
}

void PopupList::focusOutEvent(QFocusEvent* e)
{
    QPointer<PopupList> thisPtr = this;
    QMetaObject::invokeMethod(QApplication::instance(), [thisPtr]() {
        if (thisPtr)
        {
            thisPtr->finished();
        } });
    QListView::focusOutEvent(e);
}

void PopupList::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            finished();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // select the current row, which will trigger the "selected" signal
            if (currentIndex().isValid())
            {
                selectionModel()->select(currentIndex(), QItemSelectionModel::Select);
            }
            break;
        default:
            QListView::keyPressEvent(event);
            break;
    }
}

void PopupList::finished()
{
    parentWidget()->deleteLater();
}

void PopupList::adjustHeight()
{
    const int max_rows = 10;
    int row = model()->rowCount() - 1;
    if (row >= max_rows)
    {
        row = max_rows - 1;
    }
    int height = visualRect(model()->index(row, 0)).bottom() + 3;
    setFixedHeight(height);
    parentWidget()->setFixedHeight(height);
    if (model()->rowCount() > 0)
    {
        parentWidget()->show();
    }
}
