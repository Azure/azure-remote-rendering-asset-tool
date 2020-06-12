#include <Utils/FontOverrideModel.h>

FontOverrideListModel::FontOverrideListModel(const QFont& font, QAbstractItemModel* model, QObject* parent)
    : QIdentityProxyModel(parent)
    , m_font(font)
{
    setSourceModel(model);
}

QVariant FontOverrideListModel::data(const QModelIndex& proxyIndex, int role) const
{
    switch (role)
    {
        case Qt::FontRole:
            return m_font;
    }

    return QIdentityProxyModel::data(proxyIndex, role);
}
