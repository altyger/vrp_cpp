#ifndef PROCESSLOGIC_H
#define PROCESSLOGIC_H

#include <QObject>
#include <QJsonObject>
#include <QList>
#include "datatypes.h"

class ProcessLogic : public QObject
{
    Q_OBJECT
public:
    explicit ProcessLogic(QObject *parent = nullptr);

    // Fungsi utilitas (bisa dipanggil kapan saja)
    QJsonObject loadRoadData(const QString& filePath, bool& ok);

    // Fungsi untuk mengatur data sebelum proses
    void setData(const QList<TBBM>& tbbmData, const QList<SPBU>& spbuData, const QJsonObject& roadData);

public slots:
    // Slot utama yang akan dijalankan di thread
    void runMatrixCalculation();

signals:
    void progressUpdated(int value);
    void progressTextChanged(const QString& text);
    void processingFinished(bool success, const QString& message);

private:
    void buildRoadGraph();
    double haversineDistance(double lat1, double lon1, double lat2, double lon2);

    // Variabel data
    QJsonObject m_roadDataJson;
    QList<RoadNode> m_roadNodes;
    QList<RoadEdge> m_roadEdges;
    QList<TBBM> m_tbbmData;
    QList<SPBU> m_spbuData;
};

#endif // PROCESSLOGIC_H
