#include <Utils/ColorPickerButton.h>

#include <QApplication>
#include <QColorDialog>
#include <QPainter>

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

ColorPickerButton::ColorPickerButton(QWidget* parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    connect(this, SIGNAL(clicked()), this, SLOT(OpenColorDialog()));

    SetColor(QColor(255, 255, 255), false);
}

void ColorPickerButton::OpenColorDialog()
{
    clearFocus();

    {
        QColor prevColor = m_color;

        QColorDialog dlg(this);
        dlg.setCurrentColor(m_color);
        dlg.setOption(QColorDialog::ShowAlphaChannel);

        dlg.setFocus();

        connect(&dlg, &QColorDialog::currentColorChanged, this, [this](const QColor& color)
                { SetColor(color, true); });

        if (dlg.exec() == QDialog::Accepted)
        {
            SetColor(dlg.currentColor(), true);
        }
        else
        {
            SetColor(prevColor, true);
        }
    }

    setFocus();
}

void ColorPickerButton::SetColor(const QColor& color, bool emitSignal)
{
    if (m_color != color)
    {
        m_color = color;
        update();

        if (emitSignal)
        {
            Q_EMIT ColorChanged(m_color);
        }
    }
}

void ColorPickerButton::paintEvent(QPaintEvent* e)
{
    // for accessibility reasons we absolutely want the focus outline to show up
    // which is a bit tricky to do when we paint everything ourselves
    // so instead, we first let the button paint itself as normal
    // and then just paint a smaller, colored rectangle on top of it
    QPushButton::paintEvent(e);

    QColor c = m_color;
    const int borderW = 3;
    QRect r = rect().adjusted(borderW, borderW, -borderW - 1, -borderW - 1);
    QPainter p(this);

    {
        p.setCompositionMode(QPainter::CompositionMode_Source);
        drawCheckerboardPattern(p, r);
    }

    // blend the color over the background
    {
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        p.drawRect(r);
    }

    // render the same thing a second time over half the rectangle, this time without alpha
    // to make sure even at low alpha values, the user can figure out the general color
    {
        c.setAlpha(255);
        p.setBrush(c);
        p.drawRect(r.adjusted(r.width() / 2, 0, 0, 0));
    }
}
