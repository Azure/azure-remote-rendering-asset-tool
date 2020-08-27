#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <View/ModelEditor/StatsPageView.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>

StatsPageView::StatsPageView(StatsPageModel* statsPageModel)
    : m_model(statsPageModel)
{
    auto* l = new QVBoxLayout(this);

    connect(m_model, &StatsPageModel::valuesChanged, this, [this]() { updateUi(); });

    auto* fl = new QFormLayout();

    QString name;
    QString units;
    std::optional<double> minValue;
    std::optional<double> maxValue;

    for (int i = 0; i < m_model->getParameterCount(); ++i)
    {
        m_model->getParameterInfo(i, name, units, minValue, maxValue);
        m_values.push_back(new QLabel(this));
        fl->addRow(name, m_values.back());

        m_units.push_back(units);
    }
    l->addLayout(fl);
}

StatsPageView::~StatsPageView()
{
}

void StatsPageView::updateUi()
{
    assert(m_model->getParameterCount() == m_values.size());
    for (int i = 0; i < m_model->getParameterCount(); ++i)
    {
        QString toPrint = QString::number(m_model->getParameter(i));
        if (!m_units[i].isEmpty())
        {
            toPrint += " " + m_units[i];
        }
        m_values[i]->setText(toPrint);
    }
}
