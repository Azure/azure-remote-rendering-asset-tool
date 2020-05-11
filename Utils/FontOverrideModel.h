#pragma once

#include <QFont>
#include <QIdentityProxyModel>

// proxy model used to just override model font
class FontOverrideListModel : public QIdentityProxyModel
{
public:
    FontOverrideListModel(const QFont& font, QAbstractItemModel* model, QObject* parent = nullptr);
    virtual QVariant data(const QModelIndex& proxyIndex, int role) const override;

protected:
    QFont m_font;
};
