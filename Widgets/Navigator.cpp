#include <QStylePainter>
#include <Widgets/Navigator.h>
#include <utility>

Navigator::Navigator(QWidget* parent)
    : QStackedWidget(parent)
{
}

bool Navigator::navigateToPage(int index)
{
    bool couldGoBack = canGoBack();
    bool res = false;

    if (!m_modalPageStack.empty())
    {
        while (!m_modalPageStack.empty())
        {
            delete m_modalPageStack.top();
            m_modalPageStack.pop();
        }
    }
    if (m_pageHistory.empty() || m_pageHistory.top() != index)
    {
        m_pageHistory.push(index);
        res = navigateToPage_internal(index);
    }

    if (couldGoBack != canGoBack())
    {
        canGoBackChanged();
    }
    return res;
}


void Navigator::setPageFactory(PageFactory factory)
{
    m_pageFactory = std::move(factory);
}


bool Navigator::navigateToPage_internal(int index)
{
    QWidget* newPage = nullptr;
    auto pageIterator = m_ownedPages.find(index);
    if (pageIterator == m_ownedPages.end())
    {
        //not found. tries to create the page with the factory
        if (m_pageFactory)
        {
            newPage = m_pageFactory(index);
            addPage(newPage, index);
        }
    }
    else
    {
        newPage = pageIterator->second;
    }
    if (newPage)
    {
        setCurrentWidget(newPage);
        pageNavigated(index);
        return true;
    }
    else
    {
        return false;
    }
}

void Navigator::addPage(QWidget* page, int index)
{
    m_ownedPages[index] = page;
    addWidget(page);
}

void Navigator::modalPage(QWidget* page)
{
    bool couldGoBack = canGoBack();

    m_modalPageStack.push(page);
    addWidget(page);
    setCurrentWidget(page);

    if (couldGoBack != canGoBack())
    {
        canGoBackChanged();
    }
}

bool Navigator::hasPage(int index) const
{
    return m_ownedPages.count(index) > 0;
}

bool Navigator::canGoBack() const
{
    return m_pageHistory.size() > 1 || !m_modalPageStack.empty();
}

void Navigator::back()
{
    bool couldGoBack = canGoBack();

    if (!m_modalPageStack.empty())
    {
        removeWidget(m_modalPageStack.top());
        delete m_modalPageStack.top();
        m_modalPageStack.pop();

        if (!m_modalPageStack.empty())
        {
            setCurrentWidget(m_modalPageStack.top());
        }
        else if (!m_pageHistory.empty())
        {
            int lastIndex = m_pageHistory.top();
            navigateToPage_internal(lastIndex);
        }
    }
    else if (!m_pageHistory.empty())
    {
        m_pageHistory.pop();
        if (!m_pageHistory.empty())
        {
            navigateToPage_internal(m_pageHistory.top());
        }
    }

    if (couldGoBack != canGoBack())
    {
        canGoBackChanged();
    }
}

Navigator* Navigator::getNavigator(QWidget* w)
{
    for (; w != nullptr; w = w->parentWidget())
    {
        if (auto* nav = qobject_cast<Navigator*>(w))
        {
            return nav;
        }
    }
    return nullptr;
}
