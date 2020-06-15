#include <QAbstractItemView>
#include <QAccessibleWidget>
#include <QComboBox>
#include <QLineEdit>
#include <View/ArrtAccessibility.h>

QString qt_accHotKey(const QString& text);
QString qt_accStripAmp(const QString& text);


namespace ArrtAccesibility
{
    /*
    The default accessible role for QComboBox is QAccessible::ComboBox, which is exposed to UI automation
    as "Combo box". Microsoft accessibility compliance expect combo boxes to implement the ExpandCollapse interface
    and the Selection interface, see
    https://docs.microsoft.com/en-us/dotnet/framework/ui-automation/ui-automation-support-for-the-combobox-control-type
    but that's not the case in the Qt implementation.
    Furthermore, the value list exposed as a child of the combo box, doesn't have a bounding box property when the
    combobox is not expanded, and this triggers another compliance issue.
    The workaround implemented here is to override the QAccessibleInterface for QComboBox so that it returns
    the role "Button" and only exposes its children when the combo box is expanded.
    */

    // Implementation of the combo box QAccessibleInterface.
    // Based on QAccessibleComboBox in [Qt5.13.1]\Src\qtbase\src\widgets\accessible\complexwidgets_p.h, which is
    // private and can't be simply subclassed.
    // This class differs from  QAccessibleComboBox in the constructor, childCount, childAt, indexOfChild
    class ArrtAccessibleComboBox : public QAccessibleWidget
    {
    public:
        explicit ArrtAccessibleComboBox(QWidget* w);

        int childCount() const override;
        QAccessibleInterface* childAt(int x, int y) const override;
        int indexOfChild(const QAccessibleInterface* child) const override;
        QAccessibleInterface* child(int index) const override;

        QString text(QAccessible::Text t) const override;

        // QAccessibleActionInterface
        QStringList actionNames() const override;
        QString localizedActionDescription(const QString& actionName) const override;
        void doAction(const QString& actionName) override;
        QStringList keyBindingsForAction(const QString& actionName) const override;

        bool isOpen() const;

    protected:
        QComboBox* comboBox() const;
    };

    QAccessibleInterface* factory(const QString& classname, QObject* object)
    {
        if (classname == QLatin1String("QComboBox") && object && object->isWidgetType())
        {
            return new ArrtAccessibleComboBox(static_cast<QWidget*>(object));
        }
        return nullptr;
    }



    /*!
      Constructs a QAccessibleComboBox object for \a w.
    */
    ArrtAccessibleComboBox::ArrtAccessibleComboBox(QWidget* w)
        : QAccessibleWidget(w, QAccessible::Button)
    {
        Q_ASSERT(comboBox());
    }

    bool ArrtAccessibleComboBox::isOpen() const
    {
        return comboBox()->view()->isVisibleTo(comboBox());
    }

    /*!
      Returns the combobox.
    */
    QComboBox* ArrtAccessibleComboBox::comboBox() const
    {
        return qobject_cast<QComboBox*>(object());
    }


    QAccessibleInterface* ArrtAccessibleComboBox::child(int index) const
    {
        if (isOpen() && index == 0)
        {
            QAbstractItemView* view = comboBox()->view();
            return QAccessible::queryAccessibleInterface(view);
        }
        else
        {
            if (comboBox()->isEditable())
            {
                return QAccessible::queryAccessibleInterface(comboBox()->lineEdit());
            }
        }
        return 0;
    }

    int ArrtAccessibleComboBox::childCount() const
    {
        int n = 0;
        // text edit
        if (comboBox()->isEditable())
        {
            n++;
        }
        // item list
        if (comboBox()->view()->isVisibleTo(comboBox()))
        {
            n++;
        }

        return n;
    }

    QAccessibleInterface* ArrtAccessibleComboBox::childAt(int x, int y) const
    {
        if (comboBox()->isEditable() && comboBox()->lineEdit()->rect().contains(x, y))
            return child(isOpen() ? 1 : 0);
        return 0;
    }

    int ArrtAccessibleComboBox::indexOfChild(const QAccessibleInterface* child) const
    {
        bool isComboOpen = isOpen();
        if (comboBox()->view() == child->object())
        {
            return isComboOpen ? 0 : -1;
        }
        else if (comboBox()->isEditable() && comboBox()->lineEdit() == child->object())
        {
            return isComboOpen ? 1 : 0;
        }
        return -1;
    }

    /*! \reimp */
    QString ArrtAccessibleComboBox::text(QAccessible::Text t) const
    {
        QString str;

        switch (t)
        {
            case QAccessible::Name:
#ifndef Q_OS_UNIX // on Linux we use relations for this, name is text (fall through to Value)
                str = QAccessibleWidget::text(t);
                break;
#endif
            case QAccessible::Value:
                if (comboBox()->isEditable())
                    str = comboBox()->lineEdit()->text();
                else
                    str = comboBox()->currentText();
                break;
#ifndef QT_NO_SHORTCUT
            case QAccessible::Accelerator:
                str = QKeySequence(Qt::Key_Down).toString(QKeySequence::NativeText);
                break;
#endif
            default:
                break;
        }
        if (str.isEmpty())
            str = QAccessibleWidget::text(t);
        return str;
    }

    QStringList ArrtAccessibleComboBox::actionNames() const
    {
        return QStringList() << showMenuAction() << pressAction();
    }

    QString ArrtAccessibleComboBox::localizedActionDescription(const QString& actionName) const
    {
        if (actionName == showMenuAction() || actionName == pressAction())
            return QComboBox::tr("Open the combo box selection popup");
        return QString();
    }

    void ArrtAccessibleComboBox::doAction(const QString& actionName)
    {
        if (actionName == showMenuAction() || actionName == pressAction())
        {
            if (comboBox()->view()->isVisible())
            {
                comboBox()->hidePopup();
            }
            else
            {
                comboBox()->showPopup();
            }
        }
    }

    QStringList ArrtAccessibleComboBox::keyBindingsForAction(const QString& /*actionName*/) const
    {
        return QStringList();
    }
} // namespace ArrtAccesibility
