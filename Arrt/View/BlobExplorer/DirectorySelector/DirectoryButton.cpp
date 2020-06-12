#include <QKeyEvent>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QVBoxLayout>
#include <ViewUtils/DpiUtils.h>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/DirectorySelector/DirectoryButton.h>
#include <View/BlobExplorer/DirectorySelector/PopupList.h>

DirectoryButton::DirectoryButton()
    : FlatButton("")
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void DirectoryButton::setDirectory(QString directory)
{
    m_directory = std::move(directory);
    QString text = m_directory.mid(m_directory.lastIndexOf("/", -2) + 1);
    if (text.isEmpty())
    {
        text = tr("[root]");
    }
    else if (text.endsWith('/'))
    {
        text.chop(1);
    }
    setText(text);
}

QString DirectoryButton::getDirectory() const
{
    return m_directory;
}

QSize DirectoryButton::sizeHint() const
{
    return FlatButton::sizeHint() + QSize(m_buttonSize, 0);
}

QSize DirectoryButton::minimumSizeHint() const
{
    QSize sh = FlatButton::minimumSizeHint();
    const int minimumWidth = m_buttonSize + 80;
    if (sh.width() > minimumWidth)
    {
        sh.setWidth(minimumWidth);
    }
    return sh;
}

void DirectoryButton::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QString btnText;
    btnText.swap(opt.text);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);

    QRect popupButtonRect = getPopupButtonRect();
    QRect rect = opt.rect;
    rect.setRight(popupButtonRect.left());

    p.setPen(palette().text().color());
    p.setFont(opt.font);
    int alignment = Qt::AlignVCenter | Qt::AlignHCenter;
    p.drawText(rect, alignment, opt.fontMetrics.elidedText(btnText, Qt::ElideRight, rect.width(), alignment));

    const int marginLine = popupButtonRect.height() / 4;
    p.setPen(palette().dark().color());
    p.drawLine(popupButtonRect.left(), popupButtonRect.top() + marginLine, popupButtonRect.left(), popupButtonRect.bottom() - marginLine);

    QIcon icon;
    if (m_popup != nullptr)
    {
        icon = ArrtStyle::s_expandedIcon;
    }
    else
    {
        icon = ArrtStyle::s_notexpandedIcon;
    }
    QRect iconRect(0, 0, DpiUtils::size(m_expandIconSize), DpiUtils::size(m_expandIconSize));
    iconRect.moveCenter(popupButtonRect.center());
    p.drawPixmap(iconRect, icon.pixmap(iconRect.size()));
}

void DirectoryButton::mousePressEvent(QMouseEvent* me)
{
    if (getPopupButtonRect().contains(me->pos()))
    {
        if (m_popup)
        {
            hidePopup();
        }
        else
        {
            Q_EMIT popupButtonPressed();
        }
    }
    else
    {
        Q_EMIT dirPressed();
    }
}

void DirectoryButton::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Down)
    {
        if (m_popup)
        {
            m_popup->setFocus();
        }
        else
        {
            Q_EMIT popupButtonPressed();
        }
    }
    else
    {
        FlatButton::keyPressEvent(event);
    }
}

void DirectoryButton::hidePopup()
{
    m_popup->hide();
    m_popup->deleteLater();
    m_popup = {};
}


void DirectoryButton::showPopup(QAbstractItemModel* model)
{
    if (m_popup)
    {
        hidePopup();
    }
    m_popup = new QWidget(nullptr, Qt::Popup);
    m_popup->resize(200, 1);
    m_popup->move(this->mapToGlobal(getPopupButtonRect().bottomLeft()));
    m_popup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    // update on deletion, because the arrow changes direction
    m_popup->connect(m_popup, &QWidget::destroyed, this, [this] { update(); });
    m_popup->setContentsMargins(0, 0, 0, 0);
    m_popup->setMinimumHeight(1);
    auto* l = new QVBoxLayout(m_popup);
    l->setContentsMargins(0, 0, 0, 0);
    auto* list = new PopupList(model, m_popup);
    m_popup->setFocusProxy(list);

    l->addWidget(list);
    m_popup->setFocusPolicy(Qt::StrongFocus);
    m_popup->setFocus();

    connect(list, &PopupList::selected, this, &DirectoryButton::popupDirectorySelected);
    update();
}

QRect DirectoryButton::getPopupButtonRect() const
{
    QRect r = rect();
    r.setLeft(r.right() - DpiUtils::size(m_buttonSize));
    return r;
}
