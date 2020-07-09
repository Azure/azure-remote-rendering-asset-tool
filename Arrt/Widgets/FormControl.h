#pragma once

#include <QLabel>
#include <QWidget>

// wrapper on a control widget, when displayed in a form. It has a "header" (a label on top),
// to mimic the layout of XAML input controls

class FormControl : public QWidget
{
    Q_OBJECT
public:
    FormControl(const QString& header, QWidget* control, QWidget* parent = nullptr);
    FormControl(const QString& header, QLayout* layout, QWidget* parent = nullptr);
    FormControl(QWidget* parent = nullptr);
    QString getHeader() const;
    void setHeader(const QString& header);
    QWidget* getWidget() const;
    void setWidget(QWidget* control);
    QLayout* getLayout() const;
    void setLayout(QLayout* control);
    using QWidget::setToolTip;
    void setToolTip(const QString& title, const QString& details);

	// used to 
	static QObject* installFocusListener(QApplication* application);

	virtual void paintEvent(QPaintEvent* e) override;

private:
    QLabel* m_header;
    bool m_highlighted = false;
    void clear();

	void setHighlight(bool highlight);	
};
