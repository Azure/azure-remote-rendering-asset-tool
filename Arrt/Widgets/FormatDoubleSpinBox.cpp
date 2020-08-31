#include <QApplication>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QMouseEvent>
#include <QScreen>
#include <QStylePainter>
#include <QTimer>
#include <QWindow>
#include <View/ArrtStyle.h>
#include <Widgets/FormatDoubleSpinBox.h>

FormatDoubleSpinBox::FormatDoubleSpinBox(QWidget* parent, QString format, NumberFormatter::FormatterType type)
    : QDoubleSpinBox(parent)
    , m_formatter(std::move(format), type)
{
    //QhkEditorStyle::getInstance()->applyPaletteToWidget(this, "TextField");
    setFocusPolicy(Qt::StrongFocus);
    setDecimals(DBL_MAX_10_EXP + DBL_DIG);
    setMinimum(std::numeric_limits<float>().min());
    setMaximum(std::numeric_limits<float>().max());
    lineEdit()->installEventFilter(this);
    //leave the tracking false, otherwise the programmatic sets would interfere with the editing (resetting the value while editing)
    setKeyboardTracking(false);

    m_editTimer = new QTimer(this);
    m_editTimer->setSingleShot(true);
    QObject::connect(m_editTimer, SIGNAL(timeout()), this, SLOT(endEdit()));

    QObject::connect(this, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this]() {
        if (m_editHappening)
        {
            edited();
        }
    });

    QObject::connect(lineEdit(), &QLineEdit::textEdited, this, [this]() {
        m_edited = lineEdit()->isUndoAvailable();
    });

    QObject::connect(lineEdit(), &QLineEdit::returnPressed, this, [this]() {
        if (!m_editHappening)
        {
            m_edited = false;
            m_editHappening = true;
            beforeEdit();
            edited();
            afterEdit();
            m_editHappening = false;
        }
    });

    QObject::connect(lineEdit(), &QLineEdit::cursorPositionChanged, this, &FormatDoubleSpinBox::sanitizeCursorPosition);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &FormatDoubleSpinBox::onContextMenuRequested);
    endEdit(false);
}

QString FormatDoubleSpinBox::removeSuffix(const QString& s) const
{
    if (!suffix().isEmpty() && s.endsWith(suffix()))
    {
        //chops off the suffix from the string
        return s.left(s.length() - suffix().length());
    }
    else
    {
        return s;
    }
}


//used for validation during typing
QValidator::State FormatDoubleSpinBox::validate(QString& input, int& /*pos*/) const
{
    const QString stringWithoutSuffix = removeSuffix(input).trimmed();

    if (stringWithoutSuffix == "." || stringWithoutSuffix == "-" || stringWithoutSuffix == "-.")
        return QValidator::Intermediate;

    if (stringWithoutSuffix.isEmpty())
    {
        if (m_empty)
            return QValidator::Acceptable;

        return QValidator::Intermediate;
    }

    bool ok;
    const double val = m_formatter.valueFromText(stringWithoutSuffix, &ok);

    if (!ok)
    {
        return QValidator::State::Invalid;
    }
    if (m_isDiscrete)
    {
        const double modVal = fmod((val - minimum()), m_defaultStep);
        // TODO this within 0.001 bit is a bit janky
        const bool isZero = fabs(modVal - m_defaultStep) < 0.001 * m_defaultStep || modVal < 0.001 * m_defaultStep;
        if (isZero)
            return QValidator::State::Acceptable;
        else
            return QValidator::State::Intermediate;
    }

    return QValidator::Acceptable;
}

QString FormatDoubleSpinBox::textFromValue(double value) const
{
    if (m_empty)
    {
        return "";
    }
    else
    {
        return m_formatter.textFromValue(value);
    }
}

double FormatDoubleSpinBox::valueFromText(const QString& text) const
{
    QString stringWithoutSuffix = removeSuffix(text);

    bool ok;
    double val = m_formatter.valueFromText(stringWithoutSuffix, &ok);
    if (!ok)
    {
        return value();
    }
    else
    {
        return val;
    }
}

void FormatDoubleSpinBox::setEmpty(bool empty)
{
    if (m_empty != empty)
    {
        m_empty = empty;
        setValue(value());
    }
}


void FormatDoubleSpinBox::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (!m_editHappening)
        {
            // it's dragging but it's not in an edit mode. This can happen when the mousePressEvent is not called or
            // when the edit is ended by QhkExecuteBeforeShortcut (and we assume it has to be restarted by a mouseMoveEvent)
            startEdit(false);
        }


        if (!m_dragging)
        {
            m_oldCursor = cursor();
            setCursor(QCursor(Qt::SplitVCursor));
            m_dragging = true;
        }

        // This was formerly:
        // QPoint physicalPos = hktQtUtils::logicalToPhysical(event->globalPos(), window()->windowHandle()->screen());
        // The problem is that for spinboxes in visual scripts, all coordinates received from the event are relative
        // to the hidden window onto which the graphics scene paints. We read from this invisible window later to draw
        // everything ourselves.
        //      Since the coordinates are relative to that window, we have effectively no idea how to translate them to real
        // global coordinates. To get around this we simply read the cursor's position itself, which is in global
        // logical coordinates. Since in this function all we care about is the distance to the last place we clicked,
        // we don't really need to care about the widget itself anymore, and global coordinates are ok to use.
        QPoint pos = QCursor::pos(); //<restore> hktQtUtils::logicalToPhysical(QCursor::pos());

        QScreen* scr = QGuiApplication::screenAt(pos);
        QRect screen = scr->availableGeometry();
        // convert to physical screen size
        screen.setSize(screen.size() * scr->devicePixelRatio());
        screen.adjust(0, 4, 0, -4);
        int mouseY = (pos).y();
        if (mouseY < screen.top())
        {
            m_initialMouseY += screen.bottom() - mouseY;
            mouseY = screen.bottom();
            QCursor::setPos(scr, QPoint(pos.x(), mouseY));
        }
        else if (mouseY > screen.bottom())
        {
            m_initialMouseY -= mouseY - screen.top();
            mouseY = screen.top() + 1;
            QCursor::setPos(scr, QPoint(pos.x(), mouseY));
        }

        int deltaY = mouseY - m_initialMouseY;

        // convert back to logical distance so that perceived sensitivity is the same across different DPIs
        deltaY = deltaY / (float)window()->windowHandle()->devicePixelRatio();
        double step = m_dragStepPerPixel;
        if (event->modifiers() & Qt::ControlModifier)
        {
            step *= 10.0;
        }

        double roundedDelta = step * deltaY;
        if (m_isDiscrete && m_defaultStep > 0)
        {
            roundedDelta = round((roundedDelta) / m_defaultStep) * m_defaultStep;
        }

        if (m_formatter.getFormatterType() == NumberFormatter::DOUBLE_FORMAT)
        {
            setValue(m_initialValue - roundedDelta);
        }
        else
        {
            setValue((int)std::round(m_initialValue - roundedDelta));
        }

        m_edited = true;
        selectAll();
    }

    QDoubleSpinBox::mouseMoveEvent(event);
}

void FormatDoubleSpinBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        startEdit(false);

        m_initialMouseY = QCursor::pos().y();
        m_initialValue = value();
        setDragStepPerPixel(m_dragStepPerPixel);
    }
    QDoubleSpinBox::mousePressEvent(event);
    event->accept();
}

void FormatDoubleSpinBox::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_dragging)
        {
            setCursor(m_oldCursor);
        }
        endEdit(false);
        selectAll();
        m_dragging = false;
    }

    if (event->buttons() == Qt::NoButton)
    {
        releaseMouse();
    }

    QDoubleSpinBox::mouseReleaseEvent(event);
    event->accept();
}

void FormatDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    // Workaround for textEdited() signal not being sent if the QValidator makes modifications to your input.
    // https://bugreports.qt.io/browse/QTBUG-44046
    QString oldText = text();
    QDoubleSpinBox::keyPressEvent(event);
    QString newText = text();
    if (oldText != newText)
    {
        m_edited = lineEdit()->isUndoAvailable();
    }
}

void FormatDoubleSpinBox::wheelEvent(QWheelEvent* event)
{
    startEdit(true);
    QDoubleSpinBox::wheelEvent(event);
    m_edited = lineEdit()->isUndoAvailable();
    endEdit(true);
}

QSize FormatDoubleSpinBox::sizeHint() const
{
    // QDoubleSpinBox::sizeHint requires textFromValue to return useful
    // data for the min and max values to compute the size hint.
    // Therefore, we disable m_empty in this scope.
    bool bIsEmpty = m_empty;
    m_empty = false;
    QSize res = QDoubleSpinBox::sizeHint();
    if (bIsEmpty)
    {
        m_empty = true;
    }
    return res;
}

QSize FormatDoubleSpinBox::minimumSizeHint() const
{
    // QDoubleSpinBox::minimumSizeHint requires textFromValue to return useful
    // data for the min and max values to compute the minimum size hint.
    // Therefore, we disable m_empty in this scope.
    bool bIsEmpty = m_empty;
    m_empty = false;
    QSize res = QDoubleSpinBox::minimumSizeHint();
    if (bIsEmpty)
    {
        m_empty = true;
    }
    return res;
}

void FormatDoubleSpinBox::setStep(double step)
{
    if (m_formatter.getFormatterType() == NumberFormatter::INTEGER_FORMAT)
    {
        step = round(step);
    }

    m_defaultStep = step;

    if (step == 0.0)
    {
        step = 1.0;
    }
    if (singleStep() != step)
    {
        setSingleStep(step);
    }
}

void FormatDoubleSpinBox::setDragStepPerPixel(double stepPerPixel)
{
    if (stepPerPixel == 0.0f)
    {
        //auto step. 1% of the value.
        m_dragStepPerPixel = m_initialValue * 0.01;
        if (m_dragStepPerPixel < 0.0)
        {
            m_dragStepPerPixel = -m_dragStepPerPixel;
        }
        if (m_dragStepPerPixel < 0.001)
        {
            m_dragStepPerPixel = 0.001;
        }
    }
    else
    {
        m_dragStepPerPixel = stepPerPixel;
    }
}

void FormatDoubleSpinBox::setFormat(QString format, NumberFormatter::FormatterType type)
{
    m_formatter.setFormat(std::move(format), type);
}


void FormatDoubleSpinBox::setIdentityValue(double identity)
{
    m_identityValue = identity;
}

bool FormatDoubleSpinBox::event(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::Show:
        {
            lineEdit()->deselect();
            break;
        }
            //filter out the undo/redo shortcuts, to fall back on our undo handling
        case QEvent::ShortcutOverride:
        case QEvent::KeyPress:
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent == QKeySequence::Undo || keyEvent == QKeySequence::Redo)
            {
                if (!lineEdit()->isUndoAvailable())
                {
                    return false;
                }
            }
            if (keyEvent->key() == Qt::Key_Escape)
            {
                this->setValue(value());
                return true;
            }
            break;
        }
            //fix on focus (without this, the spinbox won't end the editing when losing the focus)
        case QEvent::FocusIn:
        {
            setFocusPolicy(Qt::WheelFocus);
            bool res = QDoubleSpinBox::event(event);
            selectAll();
            //see QhkLineEdit::focusInEvent
            m_selectAllOnMousePress = true;
            return res;
        }
        case QEvent::FocusOut:
        {
            setFocusPolicy(Qt::StrongFocus);
            if (m_edited)
            {
                m_edited = false;
                if (m_editHappening)
                {
                    // this can either happen as a timed edit or if the user looses focus while dragging the value of the spin box
                    endEdit();
                }
                else
                {
                    //handle first the focus out, so the value in the UI is validated/updated
                    bool retValue = QDoubleSpinBox::event(event);

                    m_editHappening = true;
                    beforeEdit();
                    edited();
                    afterEdit();
                    m_editHappening = false;

                    return retValue;
                }
            }
            break;
        }
            //ignore wheel events when not in focus
        case QEvent::Wheel:
        {
            QWidget* focused = QApplication::focusWidget();
            if (this != focused && !isAncestorOf(focused))
            {
                event->ignore();
                return false;
            }
            break;
        }
        default:
            break;
    }

    return QDoubleSpinBox::event(event);
}

QAbstractSpinBox::StepEnabled FormatDoubleSpinBox::stepEnabled() const
{
    return m_dragging ? StepNone : QAbstractSpinBox::stepEnabled();
}

bool FormatDoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == lineEdit())
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                if (m_selectAllOnMousePress)
                {
                    selectAll();
                    m_selectAllOnMousePress = false;
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }
    return false;
}

void FormatDoubleSpinBox::showEvent(QShowEvent* event)
{
    //prevents the treeview to automatically select the editor fields.
    lineEdit()->deselect();
    QDoubleSpinBox::showEvent(event);
}



void FormatDoubleSpinBox::startEdit(bool isDelayed)
{
    if (m_editHappening && !m_editTimer->isActive() && isDelayed)
    {
        //delayed edit while a non delayed edit is happening: ignore. (the non delayed one will call endEdit anyway)
        return;
    }

    // assume that only edit type that can be currently open is one that's timed
    assert(!m_editHappening || m_editTimer->isActive());

    if (isDelayed)
    {
        // if there isn't an open timed edit, start one now
        if (!m_editTimer->isActive())
        {
            beforeEdit();
        }
        // mouse wheel edits are started with a 1 second timeout
        m_editTimer->start(1000); // start/reset the timer that automatically closes the timed edit after no activity for the give time period
    }
    else
    {
        // if there's still a pending timed edit, close it now
        if (m_editHappening)
        {
            endEdit(isDelayed);
        }
        beforeEdit();
    }

    m_editHappening = true;
}

void FormatDoubleSpinBox::endEdit(bool isDelayed)
{
    if (!isDelayed) // don't end individual edits due to mouse wheel events, because they are grouped as one as long as they occur close enough together in time
    {
        //HK_ASSERT_NO_MSG(0x5f297a25, m_editHappening); // TODO: Check this, was getting triggered when the spinbox action hit a breakpoint
        if (m_editHappening)
        {
            m_edited = false;
            afterEdit();
            m_editTimer->stop();
            m_editHappening = false;
        }
    }

    m_isCancelling = false;
}


void FormatDoubleSpinBox::onContextMenuRequested(const QPoint& /*pt*/)
{
    if (m_editHappening)
        return;

    startEdit();
    setValue(m_identityValue);
    endEdit();
}

void FormatDoubleSpinBox::fixup(QString& input) const
{
    if (m_isDiscrete)
    {
        input = textFromValue(round((valueFromText(input) - minimum()) / m_defaultStep) * m_defaultStep + minimum());
    }
    else
    {
        QDoubleSpinBox::fixup(input);
    }
}

void FormatDoubleSpinBox::paintEvent(QPaintEvent* event)
{
    QDoubleSpinBox::paintEvent(event);
    if (hasFocus())
    {
        QStylePainter p(this);
        ArrtStyle::drawFocusedBorder(&p, rect());
    }
}

void FormatDoubleSpinBox::sanitizeCursorPosition(int /*oldPos*/, int newPos)
{
    if (suffix().isEmpty())
        return;

    QString s = removeSuffix(text());
    const int maxPos = s.length();

    if (newPos > maxPos)
        lineEdit()->setCursorPosition(maxPos);
}

void FormatDoubleSpinBox::cancel()
{
    m_isCancelling = true;
    endEdit(false);
}

bool FormatDoubleSpinBox::isCancelling() const
{
    return m_isCancelling;
}
