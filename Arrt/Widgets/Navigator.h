#pragma once

#include <QStackedWidget>
#include <functional>
#include <stack>
#include <unordered_map>

// Navigator widget. It has a list of possible "pages", and handles the navigation among them.
// It stores the navigation history and allows the user to go back.
// It also handles "modal pages": widgets with are not stored in the page list, but are still in the history as a stack on
// top of the last loaded page.

class Navigator : public QStackedWidget
{
    Q_OBJECT

public:
    typedef std::function<QWidget*(int index)> PageFactory;

    Navigator(QWidget* parent = nullptr);

    bool navigateToPage(int index);
    void setPageFactory(PageFactory factory);
    void addPage(QWidget* page, int index);
    void modalPage(QWidget* page);
    bool hasPage(int index) const;
    bool canGoBack() const;

    void back();

    // return the parent navigator for this widget
    static Navigator* getNavigator(QWidget* w);

signals:
    void pageNavigated(int index);
    void canGoBackChanged();

private:
    std::stack<int> m_pageHistory;
    std::stack<QWidget*> m_modalPageStack;

    std::unordered_map<int, QWidget*> m_ownedPages;

    PageFactory m_pageFactory = {};
    QWidget* m_overlay = {};

    bool navigateToPage_internal(int index);
};
