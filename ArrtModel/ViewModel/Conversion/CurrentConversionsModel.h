#pragma once
#include <Model/ConversionManager.h>
#include <QStandardItemModel>

class QStandardItem;
class QItemSelectionModel;
struct Conversion;

// list model which contains all of the current (and past) conversions

class CurrentConversionsModel : public QStandardItemModel
{
    Q_OBJECT

public:
    CurrentConversionsModel(QItemSelectionModel* selectionModel, ConversionManager* conversionManager, QObject* parent);

    enum Roles
    {
        ID = Qt::UserRole + 1, //ConversionManager::ConversionId id of the conversion
        NAME,                  // QString
        STATUS,                // Conversion::Status
        INGESTION_TIME,        // int elapsed seconds
    };

    static QString getStringFromStatus(Conversion::Status status);
    static QIcon getIconFromStatus(Conversion::Status status);

private:
    ConversionManager* const m_conversionManager;
    void addConversion(ConversionManager::ConversionId id);

    void syncConversion(QStandardItem* item);

    ConversionManager::ConversionId getConversionIdFromRow(int row) const;
    int getConversionRowFromId(ConversionManager::ConversionId row) const;

    QItemSelectionModel* const m_itemSelectionModel;
};
