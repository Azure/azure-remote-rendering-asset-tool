#include <View/ModelEditor/StatsPageView.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>
#include <QVBoxLayout>
#include <QLabel>

StatsPageView::StatsPageView(StatsPageModel* statsPageModel)
    : m_model(statsPageModel)
{
    auto* l = new QVBoxLayout(this);

	l->addWidget(new QLabel("PLACEHOLDER"));
}

StatsPageView::~StatsPageView()
{
}
