#include <App/AppWindow.h>
#include <Storage/StorageAccount.h>

void ArrtAppWindow::FileUploadStatusCallback(int numFiles)
{
    m_numFileUploads = numFiles;
    OnUpdateStatusBar();

    if (numFiles == 0)
    {
        m_storageAccount->ClearCache();
        StorageBrowser->RefreshModel();
    }
}