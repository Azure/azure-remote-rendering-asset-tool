#pragma once
#include <QListView>

// list view used for blobs
class BlobsListModel;
class ModelDelegate;

class BlobListView : public QListView
{
    Q_OBJECT
public:
    BlobListView(BlobsListModel* model, QWidget* parent = nullptr);
    void setAcceptFileDrops(bool acceptDrop);

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void filesDropped(const QStringList& files);

private:
    bool m_acceptFileDrop = false;
    BlobsListModel* const m_model;
    ModelDelegate* m_delegate = {};
};
