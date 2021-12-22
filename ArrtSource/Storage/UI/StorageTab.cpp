#include <App/AppWindow.h>

void ArrtAppWindow::FileUploadStatusCallback(int numFiles)
{
    m_numFileUploads = numFiles;
    OnUpdateStatusBar();

    if (numFiles == 0)
    {
        StorageBrowser->RefreshModel();
    }
}