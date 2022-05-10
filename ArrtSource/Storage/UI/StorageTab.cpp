#include <App/AppWindow.h>
#include <Storage/StorageAccount.h>
#include <Utils/Logging.h>

void ArrtAppWindow::FileUploadStatusCallback(int numFiles, float percentage)
{
    m_numFileUploads = numFiles;
    m_fileUploadPercentage = percentage;
    OnUpdateStatusBar();

    if (numFiles == 0)
    {
        m_storageAccount->ClearCache();
        StorageBrowser->RefreshModel();

        ScreenReaderAlert("Upload", nullptr);
        ScreenReaderAlert("Upload", "File upload finished");
    }
}