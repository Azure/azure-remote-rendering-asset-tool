#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <Utils/ScopedBlockers.h>
#include <View/ArrtStyle.h>
#include <View/Conversion/ConversionView.h>
#include <View/Conversion/InputSelectionView.h>
#include <View/Conversion/OutputSelectionView.h>
#include <View/Parameters/BoundWidget.h>
#include <View/Parameters/BoundWidgetFactory.h>
#include <ViewModel/Conversion/ConversionConfigModel.h>
#include <ViewModel/Conversion/ConversionModel.h>
#include <ViewModel/Conversion/InputSelectionModel.h>
#include <ViewModel/Conversion/OutputSelectionModel.h>
#include <ViewModel/Parameters/ParameterModel.h>
#include <ViewUtils/DpiUtils.h>
#include <ViewUtils/SimpleVerticalLayout.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>
#include <Widgets/ReadOnlyText.h>
#include <Widgets/VerticalScrollArea.h>

ConversionView::ConversionView(ConversionModel* conversionModel, QWidget* parent)
    : QWidget(parent)
    , m_model(conversionModel)
{
    setAccessibleName(tr("Selected conversion"));
    auto* l = new QVBoxLayout(this);

    auto* scrollArea = new VerticalScrollArea(this);
    l->addWidget(scrollArea, 1);

    auto* contentLayout = scrollArea->getContentLayout();

    {
        auto* nameStatus = new QWidget(scrollArea);
        nameStatus->setContentsMargins(0, 0, 0, 0);
        auto* nameStatusLayout = new QHBoxLayout(nameStatus);
        nameStatusLayout->setContentsMargins(0, 0, 0, 0);

        {
            m_name = new QLineEdit();
            auto* fc = new FormControl(tr("Name"), m_name);
            fc->setToolTip(tr("Name of the conversion"), tr("Conversions can be named by the user. If this field is left empty, it will be named with the name of the input 3D model, or by an incremental ID if that is not set."));
            nameStatusLayout->addWidget(fc);

            connect(m_name, &QLineEdit::textChanged, this, [this](const QString& s) {
                m_model->setName(s);
            });
        }

        {
            m_status = new ReadOnlyText();
            auto* fc = new FormControl(tr("Status"), m_status);
            nameStatusLayout->addWidget(fc);
        }

        contentLayout->addWidget(nameStatus);
    }

    {
        m_inputLabel = new ReadOnlyText();
        m_inputLabel->setPlaceholderText(tr("[Select input model]"));
        m_inputLabel->setAccessibleName(tr("Input model"));

        m_inputButton = new FlatButton(tr("Select"), this);
        m_inputButton->setToolTip(tr("Select input model"), tr("Open a blob explorer to find and select an input 3D model for conversion"));

        QObject::connect(m_inputButton, &FlatButton::clicked, this, [this]() {
            InputSelectionModel* model = m_model->createtInputSelectionModel();
            auto* inputSelectionView = new InputSelectionView(model);
            model->setParent(inputSelectionView);

            Navigator::getNavigator(this)->modalPage(inputSelectionView);
        });

        {
            QWidget* inputPanel = new QWidget(this);
            inputPanel->setContentsMargins(0, 0, 0, 0);
            auto* inputLayout = new QHBoxLayout(inputPanel);
            inputLayout->setContentsMargins(0, 0, 0, 0);
            inputLayout->addWidget(m_inputLabel, 1);
            inputLayout->addWidget(m_inputButton, 0);

            auto* fc = new FormControl(tr("Input 3D Model"), inputPanel);
            fc->setToolTip(tr("Input 3D model"), tr("URL of the blob of the 3D model to be converted for remote rendering"));
            contentLayout->addWidget(fc);
        }
    }

    {
        m_rootDirComboBox = new QComboBox();
        m_rootDirComboBox->setModel(m_model->getInputRootDirectorySelectorModel());

        connect(m_rootDirComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                [this](int selectedIndex) {
                    auto* model = m_model->getInputRootDirectorySelectorModel();
                    const QString newDir = model->data(model->index(selectedIndex, 0), Qt::UserRole).toString();
                    m_model->setCurrentInputRootDirectory(newDir);
                });

        auto* fc = new FormControl(tr("Input root directory"), m_rootDirComboBox);
        fc->setToolTip(tr("Input root directory"), tr("Root directory containing any data file that the model reference. This directory and its subdirectory will be copied for conversion"));
        contentLayout->addWidget(fc);

        auto updateVisibility = [this, fc]() {
            fc->setVisible(m_rootDirComboBox->count() > 1);
        };
        connect(m_model->getInputRootDirectorySelectorModel(), &QAbstractItemModel::rowsInserted, this, updateVisibility);
        connect(m_model->getInputRootDirectorySelectorModel(), &QAbstractItemModel::layoutChanged, this, updateVisibility);
        connect(m_model->getInputRootDirectorySelectorModel(), &QAbstractItemModel::modelReset, this, updateVisibility);
        connect(m_model->getInputRootDirectorySelectorModel(), &QAbstractItemModel::rowsRemoved, this, updateVisibility);
        updateVisibility();
    }


    {
        // configuration panel
        QWidget* configPanel = new QWidget(this);
        configPanel->setContentsMargins(15, 2, 2, 2);

        configPanel->setFont(ArrtStyle::s_configurationLabelFont);
        m_configLabelPalette = configPanel->palette();
        m_configLabelPalette.setColor(QPalette::WindowText, {200, 200, 200});

        auto* configLayout = new QFormLayout(configPanel);

        auto* resetButton = new FlatButton(tr("Reset"));
        resetButton->setToolTip(tr("Reset to default"), tr("Reset all of the configuration values to their default"));
        resetButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        connect(resetButton, &FlatButton::clicked, this, [this]() { m_model->resetToDefault(); });
        configLayout->addRow(createConfigLabel(tr("Reset to default")), resetButton);
        m_configControls.push_back(resetButton);

        for (const auto& controlModel : m_model->getConfigurationControls())
        {
            QWidget* w = BoundWidgetFactory::createBoundWidget(controlModel, configPanel);
            w->setFixedHeight(DpiUtils::size(20));
            w->setAccessibleName(controlModel->getName());
            if (auto* boundWidget = qobject_cast<BoundWidget*>(w))
            {
                configLayout->addRow(createConfigLabel(controlModel->getName()), w);
                m_configControls.push_back(w);
            }
            else
            {
                qWarning() << tr("Control could not be created for property") << qUtf8Printable(controlModel->getName());
            }
        }

        auto* fcLayout = new QVBoxLayout();
        auto* expandButton = new FlatButton("");
        expandButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        expandButton->setCheckable(true);
        expandButton->setChecked(false);
        auto onExpand = [expandButton, configPanel](bool isChecked) {
            expandButton->setText(QString(" ") + (isChecked ? tr("Collapse configuration") : tr("Expand configuration")));
            expandButton->setIcon(isChecked ? ArrtStyle::s_collapseIcon : ArrtStyle::s_expandIcon, true);
            expandButton->setIconSize(QSize(DpiUtils::size(10), DpiUtils::size(10)));
            configPanel->setVisible(isChecked);
        };
        connect(expandButton, &FlatButton::toggled, onExpand);
        onExpand(false);

        fcLayout->addWidget(expandButton);
        fcLayout->addWidget(configPanel);

        auto* fc = new FormControl(tr("Configuration"), fcLayout);
        fc->setToolTip(tr("Configuration for conversion"), tr("All the parameters that are used for converting the 3D model. The configuration is stored in a blob called ConversionSettings.json, stored next to the input 3D model."));
        contentLayout->addWidget(fc);
    }

    {
        m_outputLabel = new ReadOnlyText();
        m_outputLabel->setReadOnly(true);
        m_outputLabel->setPlaceholderText(tr("[Select output directory]"));
        m_outputLabel->setAccessibleName(tr("Input model"));

        m_outputButton = new FlatButton(tr("Select"), this);
        m_outputButton->setToolTip(tr("Select output directory"), tr("Open a blob explorer to select the directory where the conversion will write the converted 3D model"));

        QObject::connect(m_outputButton, &FlatButton::clicked, this, [this]() {
            OutputSelectionModel* model = m_model->createOutputSelectionModel();
            auto* outputSelectionView = new OutputSelectionView(model);
            model->setParent(outputSelectionView);
            Navigator::getNavigator(this)->modalPage(outputSelectionView);
        });

        QWidget* outputPanel = new QWidget(this);
        outputPanel->setContentsMargins(0, 0, 0, 0);
        auto* outputLayout = new QHBoxLayout(outputPanel);
        outputLayout->setContentsMargins(0, 0, 0, 0);
        outputLayout->addWidget(m_outputLabel, 1);
        outputLayout->addWidget(m_outputButton, 0);

        auto* fc = new FormControl(tr("Output"), outputPanel);
        fc->setToolTip(tr("Output directory for conversion"), tr("URL of the directory where the converted output 3D model will be created"));
        contentLayout->addWidget(fc);
    }

    {
        m_conversionId = new ReadOnlyText();
        m_conversionIdControl = new FormControl(tr("Conversion ID"), m_conversionId);
        m_conversionIdControl->setToolTip(tr("Conversion ID"), tr("ID of the current conversion"));
        contentLayout->addWidget(m_conversionIdControl);
    }


    QHBoxLayout* toolButtonsLayout = new QHBoxLayout();
    toolButtonsLayout->addStretch(1);
    m_startConversionButton = new FlatButton(tr("Start conversion"));
    m_startConversionButton->setToolTip(tr("Start conversion"), tr("Trigger the conversion using the selected configuration"));
    m_startConversionButton->setIcon(ArrtStyle::s_startIcon, true);
    QObject::connect(m_startConversionButton, &FlatButton::clicked, this, [this]() {
        m_model->startConversion();
    });
    toolButtonsLayout->addWidget(m_startConversionButton);

    l->addLayout(toolButtonsLayout);

    QObject::connect(m_model, &ConversionModel::changed, this, [this]() { updateUi(); });
    updateUi();
};


void ConversionView::updateUi()
{
    setVisible(m_model->isConversionSelected());

    {
        ScopedBlockSignals blockSignal(m_name);
        m_name->setText(m_model->getName());
    }
    m_name->setPlaceholderText(m_model->getDefaultName());
    m_name->setReadOnly(!m_model->canSetName());

    m_status->setText(m_model->getStatus());

    QString conversionId = m_model->getConversionId();
    m_conversionIdControl->setVisible(!conversionId.isEmpty());
    m_conversionId->setText(conversionId);

    m_inputLabel->setText(m_model->getInput());
    m_inputButton->setEnabled(m_model->canSelectInput());
    m_outputLabel->setText(m_model->getOutput());
    m_outputButton->setEnabled(m_model->canSelectOutput());
    int index = m_rootDirComboBox->findData(m_model->getCurrentInputRootDirectory(), Qt::UserRole);
    m_rootDirComboBox->setCurrentIndex(index);

    m_startConversionButton->setEnabled(m_model->canStartConversion());

    const bool enabled = m_model->isConfigurationEnabled();

    for (QWidget* w : m_configControls)
    {
        if (auto* bw = qobject_cast<BoundWidget*>(w))
        {
            bw->updateFromModel();
        }
        w->setEnabled(enabled);
    }
}

QLabel* ConversionView::createConfigLabel(const QString& text) const
{
    auto* label = new QLabel(text);
    label->setFont(ArrtStyle::s_configurationLabelFont);
    label->setPalette(m_configLabelPalette);
    return label;
}
