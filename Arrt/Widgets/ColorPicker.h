#pragma once

#include <QColorDialog>
#include <QPointer>
#include <QWidget>

class FormatDoubleSpinBox;

// color dialog, to select a QColor with or without alpha

class ColorDialog : public QColorDialog
{
public:
    /// Using this constructor the dialog will show a 'multiplier' field that allows to create HDR color values.
    ColorDialog(const QColor& initial, double multiplier, bool useAlpha = false);

    /// Using this constructor, no 'multiplier' field is shown
    ColorDialog(const QColor& initial, bool useAlpha = false);

    double getMultiplier()
    {
        return m_multiplier;
    }

    void setMultiplier(double multiplier);

private:
    double m_multiplier = 1.0;
    FormatDoubleSpinBox* m_multEdit;
};

// Button control showing a QColor, and opening a color dialog on click, to modify it

class ColorPicker : public QWidget
{
    Q_OBJECT

public:
    ColorPicker(QWidget* parent = nullptr);
    void setColor(const QColor& color);
    void setMultiplier(double multiplier);

    QColor getColor() const;
    double getMultiplier() const;
    void setUseAlpha(bool useAlpha);
    void setUseLinearColorSpace(bool linearColor);
    void setNotifyWhileSelecting(bool notifyWhileSelecting);
    void setUseMultiplier(bool useMultiplier);

    virtual QSize sizeHint() const override;
    void performPickValue();

protected:
    virtual void paintEvent(QPaintEvent* e) override;
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;

Q_SIGNALS:
    void beginEdit();
    void colorChanged(const QColor& color);
    void endEdit(bool canceled);

private:
    void setColorGammaSpace(const QColor& color);
    void ensureItIsEditing();

    QColor m_color;
    bool m_useAlpha = false;
    bool m_linearColor = false;
    bool m_notifyWhileSelecting = true;
    bool m_useMultiplier = false;
    double m_multiplier = 1.0;
    bool m_editing = false;
    bool m_pressed = false;
    QPointer<ColorDialog> m_colorDialog;
};
