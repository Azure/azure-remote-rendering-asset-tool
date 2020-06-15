#pragma once
#include <QPointer>
#include <Widgets/FlatButton.h>

class QAbstractItemModel;

// Custom button which represents a directory in a path. It has an arrow on its right used to open a popup to navigate to a subdirectory

class DirectoryButton : public FlatButton
{
    Q_OBJECT
public:
    DirectoryButton();
    void setDirectory(QString directory);
    void showPopup(QAbstractItemModel* model);
    void hidePopup();

    QString getDirectory() const;

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent* me) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void dirPressed();
    void popupButtonPressed();
    void popupDirectorySelected(QString dir);

private:
    QString m_directory;
    static const int m_buttonSize = 20;
    static const int m_expandIconSize = 10;
    QPointer<QWidget> m_popup;

    QRect getPopupButtonRect() const;
};
