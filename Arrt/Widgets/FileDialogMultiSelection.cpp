#include <QEvent>
#include <QListView>
#include <QPushButton>
#include <QTreeView>
#include <Widgets/FileDialogMultiSelection.h>

FileDialogMultiSelection::FileDialogMultiSelection(QWidget* parent)
    : QFileDialog(parent)
{
    setFileMode(QFileDialog::Directory);
    setOption(QFileDialog::DontUseNativeDialog, true);

    // change internal listview and treeview inside the file dialog, to allow multiple selection
    if (auto* listView = findChild<QListView*>("listView"))
    {
        listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    if (auto* treeView = findChild<QTreeView*>())
    {
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    auto pushButtons = findChildren<QPushButton*>();
    QPushButton* chooseButton = pushButtons[0];

    // override the "Choose" button to always return the selected files without any check.
    // This is needed because the default code will check the first element in the selection,
    // and when the fileMode is QFileDialog::Directory it assumes that element is a directory
    // and fail otherwise. In our case that element can be also a file.
    QObject::connect(chooseButton, &QPushButton::clicked, this, [this]() {
        // this is the "accept" behaviour of QFileDialog (see QFileDialog::accept)
        Q_EMIT filesSelected(selectedFiles());
        QDialog::accept();
    });

    // force the "Choose" button to always be enabled (it would be disabled
    // when the first selected element is a file. See previous comment)
    chooseButton->installEventFilter(this);
}

bool FileDialogMultiSelection::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        // when the element is disabled, re-enable it
        if (auto* widget = qobject_cast<QWidget*>(watched))
        {
            widget->setEnabled(true);
            return true;
        }
    }
    return false;
}
