#pragma once

#include <QPushButton>

/// A QPushButton that opens a color picker. Displays the current color on top of itself.
///
/// Used by the material editing UI.
class ColorPickerButton : public QPushButton
{
    Q_OBJECT
public:
    ColorPickerButton(QWidget* parent);

    void SetColor(const QColor& color, bool emitSignal);
    const QColor& GetColor() const { return m_color; }

Q_SIGNALS:
    void ColorChanged(const QColor& newColor);

private Q_SLOTS:
    void OpenColorDialog();

private:
    virtual void paintEvent(QPaintEvent*) override;

    QColor m_color;
};