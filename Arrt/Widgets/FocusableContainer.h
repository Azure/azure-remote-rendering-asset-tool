#pragma once
#include <QWidget>

//container around widgets, which is painted with a highlighted state if any of the children has focus

class FocusableContainer : public QWidget
{
    Q_OBJECT

public:
    FocusableContainer(QWidget* childWidget = {}, QWidget* parent = {});

    // set the child in this container. Child has to be non null and this method can only be called once
    void setChild(QWidget* child);

    static QObject* installFocusListener(QApplication* application);

    virtual void paintEvent(QPaintEvent* e) override;

private:
    bool m_highlighted = false;
    void setHighlight(bool highlight);
};
