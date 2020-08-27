#pragma once
#include <qwidget.h>

class StatsPageModel;
class QLabel;

// panel with rendering statistics

class StatsPageView : public QWidget
{
public:
    StatsPageView(StatsPageModel* statsPageModel);
    ~StatsPageView();

private:
    StatsPageModel* const m_model;
    QList<QLabel*> m_values;
    QStringList m_units;

    void updateUi();
};
