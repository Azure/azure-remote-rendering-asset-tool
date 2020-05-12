#include <QStringList>
#include <Utils/ExtensionFilter.h>
#include <cwctype>

namespace
{
    bool endsWith(const std::wstring_view& a, const std::wstring_view& b)
    {
        if (b.size() > a.size())
        {
            return false;
        }
        return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
    }
    bool endsWithNoCase(const std::wstring_view& a, const std::wstring_view& b)
    {
        if (b.size() > a.size())
        {
            return false;
        }
        return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin(), [](const auto& left, const auto& right) -> bool {
            return std::towlower(left) == std::towlower(right);
        });
    }
} // namespace

void ExtensionFilter::setAllowedExtensions(const QString& extensions)
{
    m_allowedExtensions.clear();
    for (const QString& token : extensions.split(";", QString::SkipEmptyParts))
    {
        QString t = token.trimmed();
        if (t[0] != '.')
        {
            t.prepend('.');
        }
        m_allowedExtensions.push_back(t.toStdWString());
    }
}

bool ExtensionFilter::match(const std::wstring_view& fileName) const
{
    // matches file with allowed extensions
    if (m_allowedExtensions.empty())
    {
        return true;
    }
    else
    {
        for (const auto& ext : m_allowedExtensions)
        {
            if (endsWithNoCase(fileName, ext))
            {
                return true;
            }
        }
        return false;
    }
}
