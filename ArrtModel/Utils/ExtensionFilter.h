#pragma once
#include <QString>
#include <vector>

// simple filter class used to filter file names based on extension

class ExtensionFilter
{
public:
    //semicolon separated list of extensions
    void setAllowedExtensions(const QString& extensions);

    // return true if fileName ends with one of the extensions
    bool match(const std::wstring_view& fileName) const;

private:
    std::vector<std::wstring> m_allowedExtensions;
};
