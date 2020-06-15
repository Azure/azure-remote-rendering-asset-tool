#include <Widgets/ColorPicker.h>
#include <Widgets/FormatDoubleSpinBox.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStylePainter>

ColorDialog::ColorDialog(const QColor& initial, double multiplier, bool useAlpha)
    : QColorDialog(initial)
{
    //hack in the multiplier
    QVBoxLayout* vLayout = qobject_cast<QVBoxLayout*>(layout());
    QHBoxLayout* topLayout = qobject_cast<QHBoxLayout*>(vLayout->itemAt(0)->layout());
    QVBoxLayout* rightLayout = qobject_cast<QVBoxLayout*>(topLayout->itemAt(1)->layout());
    QGridLayout* gl = qobject_cast<QGridLayout*>(rightLayout->itemAt(2)->widget()->layout());
    QLabel* multLab = new QLabel(tr("Multiplier:"));
    multLab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_multEdit = new FormatDoubleSpinBox();
    m_multEdit->setMinimumWidth(20);
    m_multEdit->setMinimumWidth(20);
    m_multEdit->setMinimum(1.0);
    m_multEdit->setMaximum(100000.0);
    gl->addWidget(multLab, 4, 1, 1, 3);
    gl->addWidget(m_multEdit, 4, 4);
    m_multiplier = multiplier;
    m_multEdit->setValue(m_multiplier);

    if (useAlpha)
    {
        setOptions(QColorDialog::ShowAlphaChannel);
        setCurrentColor(initial);
    }

    QObject::connect(m_multEdit, &FormatDoubleSpinBox::edited, this, [this]() {
        setMultiplier(m_multEdit->value());
    });
}

ColorDialog::ColorDialog(const QColor& initial, bool useAlpha)
    : QColorDialog(initial)
{
    if (useAlpha)
    {
        setOptions(QColorDialog::ShowAlphaChannel);
        setCurrentColor(initial);
    }
    m_multEdit = nullptr;
}

void ColorDialog::setMultiplier(double multiplier)
{
    m_multiplier = multiplier;
    if (m_multEdit)
    {
        m_multEdit->setValue(m_multiplier);
    }
    currentColorChanged(currentColor());
}

ColorPicker::ColorPicker(QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setMinimumHeight(20);
    setMinimumWidth(20);

    setCursor(Qt::PointingHandCursor);
}

void ColorPicker::setColorGammaSpace(const QColor& color)
{
    m_color = color;

    update();
    colorChanged(m_color);

    setToolTip(tr("Red: %1, Green: %2, Blue: %3, Alpha: %4").arg(m_color.red()).arg(m_color.green()).arg(m_color.blue()).arg(m_color.alpha()));
}

void ColorPicker::setColor(const QColor& color)
{
    /*
   <TODO>
    if (m_linearColor)
    {
        // Convert from linear to srgb (internal storage). This sacrifices precision as the color picker only supports 8bit colors,
        // but is the only way to do it for now.
        hkColorUbGamma colorGamma(hkColorf(color.redF(), color.greenF(), color.blueF()));
        m_color.setRgb(colorGamma.r, colorGamma.g, colorGamma.b);
    }
    else */
    {
        m_color = color;
    }

    update();
    colorChanged(m_color);

    setToolTip(tr("Red: %1, Green: %2, Blue: %3, Alpha: %4").arg(m_color.red()).arg(m_color.green()).arg(m_color.blue()).arg(m_color.alpha()));
}

void ColorPicker::setMultiplier(double multiplier)
{
    m_useMultiplier = true;
    m_multiplier = multiplier;
}

void ColorPicker::setUseMultiplier(bool useMultiplier)
{
    m_useMultiplier = useMultiplier;
}

QColor ColorPicker::getColor() const
{
    /*
    <TODO>
    if (m_linearColor)
    {
        // Convert from srgb (internal storage) to linear. This sacrifices precision as the color picker only supports 8bit colors,
        // but is the only way to do it for now.
        hkColorf colorLinear(hkColorUbGamma(m_color.red(), m_color.green(), m_color.blue()));
        QColor color;
        color.setRgbF(colorLinear.getRed(), colorLinear.getGreen(), colorLinear.getBlue());

        return color;
    }
    */

    return m_color;
}

double ColorPicker::getMultiplier() const
{
    return m_multiplier;
}

void ColorPicker::setUseAlpha(bool useAlpha)
{
    if (m_useAlpha != useAlpha)
    {
        m_useAlpha = useAlpha;
        update();
    }
}

void ColorPicker::setUseLinearColorSpace(bool linearColor)
{
    if (m_linearColor != linearColor)
    {
        m_linearColor = linearColor;
        update();
    }
}

void ColorPicker::setNotifyWhileSelecting(bool notifyWhileSelecting)
{
    m_notifyWhileSelecting = notifyWhileSelecting;
}

QSize ColorPicker::sizeHint() const
{
    return QSize(40, 20);
}


static void drawCheckerboardPattern(QPainter& painter, const QRect& rect)
{
    static bool bInitialized = false;
    static QPixmap pattern;

    if (!bInitialized)
    {
        // Build grid pattern.
        const int iSize = 16;
        QImage img(iSize, iSize, QImage::Format::Format_RGBA8888);
        img.fill(Qt::white);
        constexpr QRgb halfGrayColor = qRgb(191, 191, 191);
        for (int i = 0; i < iSize / 2; ++i)
        {
            for (int j = 0; j < iSize / 2; ++j)
            {
                img.setPixel(i, j, halfGrayColor);
                img.setPixel(i + iSize / 2, j + iSize / 2, halfGrayColor);
            }
        }
        pattern = QPixmap::fromImage(img);
    }

    painter.drawTiledPixmap(rect, pattern);
}


void ColorPicker::paintEvent(QPaintEvent* /*e*/)
{
    QColor c = m_color;
    QRect rect = geometry();
    rect.moveTopLeft(QPoint(0, 0));
    rect.setBottomRight(rect.bottomRight() - QPoint(1, 1));
    QStylePainter p(this);

    if (m_useAlpha)
    {
        // when alpha is needed, first render a checkerboard background pattern
        p.setCompositionMode(QPainter::CompositionMode_Source);
        drawCheckerboardPattern(p, rect);
    }
    else
    {
        c.setAlpha(255);
    }

    // blend the color over the background
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawRect(rect);


    // if alpha is enabled, render the same thing a second time over half the rectangle, this time without alpha
    // to make sure even at low alpha values, the user can figure out the general color
    if (m_useAlpha)
    {
        c.setAlpha(255);
        p.setBrush(c);
        rect.adjust(0, 0, 0, -rect.height() / 2);
        p.drawRect(rect);
    }

    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect);
}


void ColorPicker::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && contentsRect().contains(e->localPos().toPoint()))
    {
        m_pressed = true;
        e->accept();
    }
}

void ColorPicker::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_pressed && e->button() == Qt::LeftButton)
    {
        m_pressed = false;
        if (contentsRect().contains(e->localPos().toPoint()))
        {
            performPickValue();
        }
    }
}

void ColorPicker::ensureItIsEditing()
{
    if (!m_editing)
    {
        m_editing = true;
        beginEdit();
    }
}

void ColorPicker::performPickValue()
{
    if (isEnabled())
    {
        ensureItIsEditing();

        QPointer<ColorDialog> cd = m_useMultiplier ? new ColorDialog(m_color, m_multiplier, m_useAlpha) : new ColorDialog(m_color, m_useAlpha);
        QMetaObject::Connection con = QObject::connect(this, &QObject::destroyed, this,
                                                       [cd]() {
                                                           if (cd)
                                                           {
                                                               cd->deleteLater();
                                                           }
                                                       });

        // If 'this' gets deleted, any of the lambda connections will still fire and run into invalid access if we don't guard against it.
        QPointer<ColorPicker> thisGuard = this;

        // Due to event queuing it might happen that we get a color change event after a shortcut has been triggered (which should stop all actions)
        // Therefore, we need this guard against it so that we don't restart the color editing after the shortcut.
        bool shortcutTriggered = false;
        /*
        new QhkExecuteBeforeShortcut(cd,
            [thisGuard, cd, &shortcutTriggered]()
        {
            // end the editing before executing an external shortcut action (like save)
            if (thisGuard && thisGuard->m_editing && cd)
            {
                thisGuard->endEdit(false);  // Does not count as cancellation.
                thisGuard->m_editing = false;
                shortcutTriggered = true;
                cd->accept();
            }
        }//);
        */

        if (m_notifyWhileSelecting)
        {
            QObject::connect(cd, &QColorDialog::currentColorChanged, this, [thisGuard, cd, &shortcutTriggered](const QColor& color) {
                if (thisGuard && !shortcutTriggered)
                {
                    thisGuard->ensureItIsEditing();
                    if (thisGuard->m_notifyWhileSelecting)
                    {
                        if (thisGuard->m_useMultiplier)
                        {
                            thisGuard->m_multiplier = cd->getMultiplier();
                        }
                        thisGuard->setColorGammaSpace(color);
                    }
                }
            });
        }

        QObject::connect(cd, &QColorDialog::colorSelected, this, [thisGuard, cd, &shortcutTriggered](const QColor& color) {
            if (thisGuard && !shortcutTriggered)
            {
                thisGuard->ensureItIsEditing();
                if (thisGuard->m_useMultiplier)
                {
                    thisGuard->m_multiplier = cd->getMultiplier();
                }
                thisGuard->setColorGammaSpace(color);
            }
        });

        QColor oldColor = m_color;
        int res = cd->exec();
        if (!thisGuard)
        {
            return;
        }

        QObject::disconnect(con);
        if (res == QDialog::Rejected)
        {
            if (m_useMultiplier)
            {
                m_multiplier = cd->getMultiplier();
            }
            setColorGammaSpace(oldColor);
        }
        delete cd;

        // Editing might be false in the shortcut case.
        if (!shortcutTriggered)
        {
            endEdit(res == QDialog::Rejected);
            m_editing = false;
        }
    }
}
