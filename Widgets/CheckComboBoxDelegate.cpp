#include <QEvent>
#include <QKeyEvent>
#include <Widgets/CheckComboBox.h>
#include <Widgets/CheckComboBoxDelegate.h>

// CheckBoxItem

CheckBoxItem::CheckBoxItem(QWidget* parent)
    : QCheckBox(parent)
{
    setTristate();
}


void CheckBoxItem::nextCheckState()
{
    // Do not allow tri-states through direct user interaction. Only by specifically setting the state.
    if ((Qt::CheckState)((checkState() + 1) % 3) == Qt::PartiallyChecked)
    {
        // In case the check state is currently partially checked, set it to the next state
        setCheckState((Qt::CheckState)((checkState() + 2) % 3));
    }
    else
    {
        QCheckBox::nextCheckState();
    }
}

CheckComboBoxDelegate::CheckComboBoxDelegate(QObject* parent, CheckComboBox* /*comboBox*/)
    : CustomComboBoxDelegate(parent)
{
}

QWidget* CheckComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    if (isSeparator(index))
    {
        return nullptr;
    }


    // create check box as our editor
    CheckBoxItem* editor = new CheckBoxItem(parent);

    //there is no easy work around on the fact that commitData is non const, and we need to do the binding
    //with the editor, from a const method. CommitData is a signal and should not have any side effect on the delegate, anyway
    CheckComboBoxDelegate* thisNonConst = const_cast<CheckComboBoxDelegate*>(this);
    connect(editor, &QCheckBox::stateChanged, this, [thisNonConst, editor]() {
        thisNonConst->commitData(editor);
    });

    // This margin mimics the margin used in the popup to draw the element.
    // It needs to be the same, or when hovering on the element, the editor will be misaligned
    editor->setStyleSheet("margin-left: 3px;");

    return editor;
}

void CheckComboBoxDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    CustomComboBoxDelegate::initStyleOption(option, index);


    if (!isSeparator(index))
    {
        option->features |= QStyleOptionViewItem::HasCheckIndicator;
    }
}

void CheckComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // Set editor data
    QVariant checkState = index.data(Qt::CheckStateRole);
    CheckBoxItem* checkBox = static_cast<CheckBoxItem*>(editor);
    checkBox->setCheckState(static_cast<Qt::CheckState>(checkState.value<int>()));
}

void CheckComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    CheckBoxItem* checkBox = static_cast<CheckBoxItem*>(editor);
    model->setData(index, checkBox->checkState(), Qt::CheckStateRole);
}

void CheckComboBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    editor->setGeometry(option.rect);
}

bool CheckComboBoxDelegate::eventFilter(QObject* receiver, QEvent* event)
{
    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
    {
        return false;
    }
    return CustomComboBoxDelegate::eventFilter(receiver, event);
}
