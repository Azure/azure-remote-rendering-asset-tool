#pragma once

#include <QCursor>
#include <QDoubleSpinBox>
#include <Utils/Formatter.h>

class QTimer;

// Double spin box with format and mouse-drag spin capabilities.
// Can be used for both double and int values as an int can fit in a double and
// the formating can be set to int formatting as well.
class FormatDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit FormatDoubleSpinBox(QWidget* parent = nullptr, QString format = {}, NumberFormatter::FormatterType type = NumberFormatter::DOUBLE_FORMAT);

    virtual QString textFromValue(double value) const override;
    virtual double valueFromText(const QString& text) const override;
    virtual QValidator::State validate(QString& input, int& pos) const override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual bool event(QEvent* event) override;

    //0.0 means it's an automatic step (proportional to the value)
    void setStep(double step);
    void setDragStepPerPixel(double stepPerPixel);
    void setFormat(QString format, NumberFormatter::FormatterType type);
    void setIdentityValue(double identity);
    void setDiscrete(bool isDiscrete) { m_isDiscrete = isDiscrete; }
    //set the widget in a state for which it only displays an empty string
    void setEmpty(bool empty);
    bool isEmpty() { return m_empty; }

    void cancel();
    bool isCancelling() const;

    QLineEdit* getLineEdit() { return lineEdit(); }

Q_SIGNALS:
    void beforeEdit();
    void afterEdit();
    void edited();

protected:
    virtual QAbstractSpinBox::StepEnabled stepEnabled() const override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;
    virtual void fixup(QString& input) const override;
protected Q_SLOTS:
    void startEdit(bool isDelayed = false);
    void endEdit(bool isDelayed = false);
    void onContextMenuRequested(const QPoint& pt);
    void sanitizeCursorPosition(int oldPos, int newPos);

protected:
    QTimer* m_editTimer;
    mutable bool m_editHappening = false;

    QString removeSuffix(const QString& s) const;

private:
    double m_defaultStep = 0.0;
    double m_identityValue = 0.0;

    // Controls whether the value must be discretized to increments of m_defaultStep
    bool m_isDiscrete = false;

    //dragging active (click on arrows, and move up/down)
    bool m_dragging = false;
    //mouse Y at the beginning of the drag
    int m_initialMouseY = 0;
    //value at the beginning of the drag
    double m_initialValue = 0.0;
    //step used in the current drag
    double m_dragStepPerPixel = 0.0;
    //cursor before dragging
    QCursor m_oldCursor;
    //formatter (handles conversions string<->number)
    NumberFormatter m_formatter;
    //when true, a mouse press will do a select all
    bool m_selectAllOnMousePress = false;
    //when the widget is empty, it only displays empty string
    mutable bool m_empty = false;

    bool m_isCancelling = false;

    bool m_edited = false;
};
