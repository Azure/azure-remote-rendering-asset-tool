#pragma once
#include <Model/ARRServiceStats.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QAbstractItemModel>
#include <optional>


// model for the stats panel, holding the statistics per frame

class StatsPageModel : public QObject
{
    Q_OBJECT

public:
    StatsPageModel(ArrServiceStats* serviceStats, QObject* parent = nullptr);

    int getParameterCount() const;
    void getParameterInfo(int index, QString& name, QString& units, std::optional<double>& minValue, std::optional<double>& maxValue) const;
    double getParameter(int index) const;
    void getGraphData(int index, std::vector<QPointF>& graph) const;
    std::optional<RR::PerformanceRating> getParameterRating(int index) const;

Q_SIGNALS:
    void valuesChanged();

private:
    ArrServiceStats* const m_serviceStats;
    ArrServiceStats::Stats m_stats;
};
