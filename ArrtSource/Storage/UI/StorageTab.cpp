#include <App/AppWindow.h>

void ArrtAppWindow::FileUploadStatusCallback(int numFiles, bool /*hadErrors*/)
{
    m_numFileUploads = numFiles;
    onUpdateStatusBar();

    if (numFiles == 0)
    {
        StorageBrowser->RefreshModel();
    }
}