#include <QApplication>
#include <QPointer>
#include <QStylePainter>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <Widgets/FormControl.h>

FormControl::FormControl(const QString& header, QWidget* control, QWidget* parent)
    : FormControl(parent)
{
    setHeader(header);
    setWidget(control);
}

FormControl::FormControl(const QString& header, QLayout* l, QWidget* parent)
    : FormControl(parent)
{
    setHeader(header);
    setLayout(l);
}

FormControl::FormControl(QWidget* parent)
    : QWidget(parent)
{
    {
        m_header = new QLabel();

        m_header->setFont(ArrtStyle::s_formHeaderFont);
        QPalette p = m_header->palette();
        p.setColor(QPalette::WindowText, ArrtStyle::s_underTextColor);
        m_header->setPalette(p);
        m_header->setVisible(false);
    }

    auto* l = new QVBoxLayout(this);

    l->setMargin(0);
    l->setContentsMargins(10, 3, 10, 5);
    l->setSpacing(2);
    l->addWidget(m_header);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

QString FormControl::getHeader() const
{
    return m_header->text();
}

void FormControl::setHeader(const QString& header)
{
    m_header->setVisible(!header.isEmpty());
    m_header->setText(header);
    m_header->setToolTip(header);
    setAccessibleName(header);
}

void FormControl::setWidget(QWidget* control)
{
    clear();
    if (control != nullptr)
    {
        layout()->addWidget(control);
        if (control->accessibleName().isEmpty())
        {
            control->setAccessibleName(m_header->text());
        }
    }
}

void FormControl::setLayout(QLayout* l)
{
    clear();
    if (l != nullptr)
    {
        static_cast<QVBoxLayout*>(layout())->addLayout(l);
    }
}

void FormControl::setToolTip(const QString& title, const QString& details)
{
    m_header->setToolTip(ArrtStyle::formatToolTip(title, details));
}

QWidget* FormControl::getWidget() const
{
    if (layout()->count() > 1)
    {
        return layout()->itemAt(1)->widget();
    }
    return nullptr;
}

QLayout* FormControl::getLayout() const
{
    if (layout()->count() > 1)
    {
        return layout()->itemAt(1)->layout();
    }
    return nullptr;
}

void FormControl::clear()
{
    if (auto* w = getWidget())
    {
        delete w;
    }
    else if (auto* l = getWidget())
    {
        delete l; //is that correct?
    }
}

void FormControl::setHighlight(bool highlight)
{
    if (highlight != m_highlighted)
    {
        m_highlighted = highlight;
        update();
    }
}

class FormControlFocusListener : public QObject
{
public:
    typedef std::function<void(FormControl* formControl, bool highlight)> Callback;

    FormControlFocusListener(QApplication* application, Callback callback)
        : QObject(application)
        , m_callback(std::move(callback))
    {
        connect(application, &QApplication::focusChanged, this, [this](QWidget* /*old*/, QWidget* now) {
            FormControl* newFocusedControl = {};
            if (now)
            {
                auto* w = now;
                while (w != nullptr)
                {
                    if (newFocusedControl = qobject_cast<FormControl*>(w))
                    {
                        break;
                    }
                    w = w->parentWidget();
                }
            }
            if (newFocusedControl != m_focusedFormControl)
            {
                if (m_focusedFormControl)
                {
                    m_callback(m_focusedFormControl, false);
                }
                m_focusedFormControl = newFocusedControl;
                if (m_focusedFormControl)
                {
                    m_callback(m_focusedFormControl, true);
                }
            }
        });
    }

private:
    QPointer<FormControl> m_focusedFormControl;
    Callback m_callback;
};

QObject* FormControl::installFocusListener(QApplication* application)
{
    auto onHighlighChanged = [](FormControl* formControl, bool highlight) {
        if (formControl)
        {
            formControl->setHighlight(highlight);
        }
    };

    return new FormControlFocusListener(application, std::move(onHighlighChanged));
}

void FormControl::paintEvent(QPaintEvent* e)
{
    if (m_highlighted)
    {
        QStylePainter p(this);
        p.fillRect(rect(), ArrtStyle::s_formControlFocusedColor);
    }
    QWidget::paintEvent(e);
}
