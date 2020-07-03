#include <QHBoxLayout>
#include <View/BlobExplorer/DirectorySelector/DirectoryButton.h>
#include <View/BlobExplorer/DirectorySelector/DirectorySelector.h>
#include <View/BlobExplorer/DirectorySelector/NewFolderButton.h>
#include <View/BlobExplorer/DirectorySelector/PopupList.h>
#include <ViewModel/BlobExplorer/DirectoryProvider.h>

DirectorySelector::DirectorySelector(DirectoryProvider* directoryModel, QWidget* parent)
    : QWidget(parent)
    , m_directoryModel(directoryModel)
{
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
    auto* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    m_buttonContainer = new QWidget(this);
    m_buttonContainer->setContentsMargins(0, 0, 0, 0);
    m_buttonContainer->setFocusPolicy(Qt::TabFocus);

    m_buttonLayout = new QHBoxLayout(m_buttonContainer);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setMargin(2);
    l->addWidget(m_buttonContainer);

    auto* newFolderButton = new NewFolderButton(this);
    connect(newFolderButton, &NewFolderButton::newFolderRequested, this, [this](const QString& newFolder) {
        m_directoryModel->navigateIntoDirectory(newFolder);
    });

    l->addWidget(newFolderButton);
    l->addStretch(1);
    updateUi();
    connect(m_directoryModel, &DirectoryProvider::directoryChanged, this,
            [this]() {
                updateUi();
            });
}

void DirectorySelector::updateUi()
{
    QString directory = m_directoryModel->getDirectory();

    int buttonIndex = 0;
    int sepIndex = -1;
    while (true)
    {
        DirectoryButton* button;
        if (buttonIndex >= m_buttons.size())
        {
            button = new DirectoryButton(m_buttonContainer);
            m_buttonLayout->addWidget(button);

            // fix tab order for newly created widget
            if (m_buttons.empty())
            {
                // if it's the first button then it's the proxy of the buttonContainer (which is equivalent to anchor the tab position to the parent container)
                QWidget::setTabOrder(m_buttonContainer, button);
                m_buttonContainer->setFocusProxy(button);
            }
            else
            {
                // otherwise it just appends the new button to the last button
                QWidget::setTabOrder(m_buttons.last(), button);
            }

            m_buttons.push_back(button);

            connect(button, &DirectoryButton::dirPressed, this, [this, button] {
                m_directoryModel->setDirectory(button->getDirectory());
            });
            connect(button, &DirectoryButton::popupButtonPressed, this, [this, button] {
                button->showPopup(m_directoryModel->createNextDirectoriesModel(button->getDirectory()));
            });
            connect(button, &DirectoryButton::popupDirectorySelected, this, [this](QString const& dir) {
                m_directoryModel->setDirectory(dir);
            });
        }
        else
        {
            button = m_buttons[buttonIndex];
        }
        button->setDirectory(directory.mid(0, sepIndex + 1));

        ++buttonIndex;

        sepIndex = directory.indexOf('/', sepIndex + 1);
        if (sepIndex == -1)
        {
            break;
        }
    }

    for (int i = m_buttons.size() - 1; i >= buttonIndex; --i)
    {
        delete m_buttons.back();
        m_buttons.pop_back();
    }
}
