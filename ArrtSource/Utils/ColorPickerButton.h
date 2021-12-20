#pragma once

#include <QPushButton>

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