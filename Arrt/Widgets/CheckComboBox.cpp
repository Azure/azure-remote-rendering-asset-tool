#include <Widgets/CheckComboBox.h>
#include <Widgets/CheckComboBoxDelegate.h>

#include <QAbstractItemView>
#include <QEvent>
#include <QKeyEvent>
#include <Utils/ScopedBlockers.h>

CheckComboBox::CheckComboBox(QWidget* widget)
    : CustomComboBox(widget)
    , m_viewOpen(false)
    , m_edited(false)
    , m_inModelDataChanged(false)
{
    view()->setItemDelegate(new CheckComboBoxDelegate(view(), this));

    // Enable editing for the popup.
    view()->setEditTriggers(QAbstractItemView::CurrentChanged);

    // Filter the popup events, to avoid closing on click and to handle the ESC.
    view()->viewport()->installEventFilter(this);
    view()->installEventFilter(this);
    connect(model(), &QAbstractItemModel::dataChanged, this, &CheckComboBox::onModelDataChanged);
    connect(model(), &QAbstractItemModel::rowsInserted, this, &CheckComboBox::onModelRowsInserted);
}

bool CheckComboBox::eventFilter(QObject* object, QEvent* event)
{
    if (object == view()->viewport())
    {
        switch (event->type())
        {
                // Filter out mouse button release in the popup, to avoid closing on click.
            case QEvent::MouseButtonRelease:
            {
                return true;
            }
            case QEvent::KeyPress:
            {
                // If the user presses escape, it hides the popup without committing the data ( see hidePopup() ).
                int key = static_cast<QKeyEvent*>(event)->key();
                if (key == Qt::Key_Escape)
                {
                    // Calling restoreState before endEdit will trigger data changed events so the order here is important.
                    restoreState();
                    endEdit();
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }

    if (object == view())
    {
        if (event->type() == QEvent::Show)
        {
            m_viewOpen = true;
        }
        if (event->type() == QEvent::Hide)
        {
            // hidePopup is not consistently called so we use the hide event instead.
            endEdit();
            m_viewOpen = false;
        }
    }
    return CustomComboBox::eventFilter(object, event);
}

QString CheckComboBox::getText()
{
    if (m_selectAllIndex.isValid() && getCheckState(m_selectAllIndex.row()) == Qt::Checked)
    {
        return itemData(m_selectAllIndex.row(), Qt::DisplayRole).toString();
    }

    QString text = "";
    for (int i = 0; i < this->count(); ++i)
    {
        if (itemData(i, Qt::CheckStateRole) == Qt::Checked)
        {
            if (!text.isEmpty())
            {
                text += ", ";
            }
            text += itemData(i, Qt::DisplayRole).toString();
        }
    }
    if (text.isEmpty())
    {
        text = "<none>";
    }
    return text;
}

void CheckComboBox::getOptions(QStyleOptionComboBox* opt)
{
    CustomComboBox::getOptions(opt);
    opt->currentText = getText();
    opt->currentIcon = QIcon();
}

void CheckComboBox::saveState()
{
    m_stateBeforeEdit.resize(this->count());
    for (int i = 0; i < this->count(); ++i)
    {
        if (!CheckComboBoxDelegate::isSeparator(model()->index(i, 0)))
        {
            m_stateBeforeEdit[i] = static_cast<Qt::CheckState>(itemData(i, Qt::CheckStateRole).value<int>());
        }
    }
}

void CheckComboBox::restoreState()
{
    if (!m_edited)
    {
        return;
    }

    if (m_stateBeforeEdit.size() == this->count())
    {
        for (int i = 0; i < this->count(); ++i)
        {
            if (isValidIndex(i))
            {
                // Will trigger data changed events
                setItemData(i, m_stateBeforeEdit[i], Qt::CheckStateRole);
            }
        }
    }
}

void CheckComboBox::showPopup()
{
    saveState();

    CustomComboBox::showPopup();

    // we open every editor here as a persistent editor to avoid the constant creation and deletion of
    // editors on mouse-over. It also fixes bug #8204699, which may have been caused by premature editor deletion.
    int rowCount = view()->model()->rowCount();
    for (int i = 0; i < rowCount; ++i)
    {
        view()->openPersistentEditor(view()->model()->index(i, 0));
    }
}

Qt::CheckState CheckComboBox::getCheckState(int index) const
{
    QVariant var = itemData(index, Qt::CheckStateRole);
    return static_cast<Qt::CheckState>(var.value<int>());
}

void CheckComboBox::setCheckState(int index, Qt::CheckState state)
{
    setItemData(index, state, Qt::CheckStateRole);
    if (isValidIndex(index))
    {
        updateSelectAllItemState();
    }
}

void CheckComboBox::addSelectAllItem(const QString& text)
{
    assert(!m_selectAllIndex.isValid()); //Select-All item already added to combo box

    int newIndex = count();
    addItem(text);

    m_selectAllIndex = model()->index(newIndex, 0);

    updateSelectAllItemState();
}

int CheckComboBox::getSelectAllIndex() const
{
    return m_selectAllIndex.row();
}

void CheckComboBox::startEdit()
{
    if (!m_viewOpen)
    {
        return;
    }
    if (!m_edited)
    {
        m_edited = true;
        Q_EMIT beforeEdit();
    }
}

void CheckComboBox::endEdit()
{

    // Since the data changes are committed to the model only if the current index changes,
    // when closing the popup, we force an index change by setting the current index to null.
    view()->setCurrentIndex(QModelIndex());
    CustomComboBox::hidePopup();

    if (!m_edited)
    {
        return;
    }
    Q_EMIT afterEdit();
    m_edited = false;
}

void CheckComboBox::onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& /*roles*/)
{
    if (m_inModelDataChanged)
    {
        return;
    }
    m_inModelDataChanged = true;
    startEdit();

    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        Qt::CheckState newCheckState = getCheckState(row);

        // when the change comes from qt (not from setCheckState), updateSelectAllItemState() won't be called
        // if we don't call setCheckState.

        setCheckState(row, newCheckState);

        if (row == getSelectAllIndex() && newCheckState != Qt::PartiallyChecked)
        {
            for (int idx = 0; idx < count(); ++idx)
            {
                if (isValidIndex(idx))
                {
                    setCheckState(idx, newCheckState);
                }
            }
        }
    }

    m_inModelDataChanged = false;
    dataChanged();
}

void CheckComboBox::onModelRowsInserted(const QModelIndex&
#ifndef NDEBUG
                                            parent
#endif
                                        ,
                                        int first, int last)
{
    assert(!parent.isValid());

    // Add CheckState data for each item, so that the model data doesn't fire dataChanged when the
    // popup menu is shown and the check state is set for the first time.

    {
        ScopedBlockSignals blockSignals(model());
        for (int i = first; i <= last; i++)
        {
            setCheckState(i, Qt::Unchecked);
        }
    }
}

bool CheckComboBox::isValidIndex(int index)
{
    if (index < 0)
    {
        return false;
    }
    return (!CheckComboBoxDelegate::isSeparator(model()->index(index, 0))) && index != m_selectAllIndex.row();
}

void CheckComboBox::updateSelectAllItemState()
{
    if (m_selectAllIndex.isValid())
    {
        int numChecked = 0;
        int numTotal = 0;
        for (int i = 0; i < count(); i++)
        {
            if (!isValidIndex(i))
            {
                continue;
            }

            numTotal++;
            if (getCheckState(i) == Qt::Checked)
            {
                numChecked++;
            }
        }

        if (numChecked == 0)
        {
            setCheckState(m_selectAllIndex.row(), Qt::Unchecked);
        }
        else if (numChecked == numTotal)
        {
            setCheckState(m_selectAllIndex.row(), Qt::Checked);
        }
        else
        {
            setCheckState(m_selectAllIndex.row(), Qt::PartiallyChecked);
        }
    }
}
