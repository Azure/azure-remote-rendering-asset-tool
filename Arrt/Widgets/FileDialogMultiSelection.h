#pragma once

#include <QFileDialog>

// hacked version of the non native file dialog, so that it can have multiple selection of both directories and files
class FileDialogMultiSelection : public QFileDialog
{
public:
    FileDialogMultiSelection(QWidget* parent = {});
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};
