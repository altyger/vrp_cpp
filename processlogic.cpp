#include "processlogic.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QPointF>
#include <QSet>
#include <QtMath>
#include <QHash>      // <-- PENTING
#include <QDebug>
#include <QThread>

// --- PERBAIKAN ERROR: Fungsi qHash untuk QPointF ---
// Ini adalah perbaikan untuk error kompilasi Anda. Pastikan ini ada.
inline uint qHash(const QPointF &key, uint seed)
{
    return qHash(key.x(), seed) ^ qHash(key.y(), seed);
}
// ---------------------------------------------------

ProcessLogic::ProcessLogic(QObject *parent) : QObject(parent) {}

QJsonObject ProcessLogic::loadRoadData(const QString &filePath, bool &ok)
{
    ok = false;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << filePath;
        return QJsonObject();
    }
    QByteArray fileData = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }
    if (!jsonDoc.isObject()) {
        qWarning() << "JSON document is not an object.";
        return QJsonObject();
    }
    ok = true;
    return jsonDoc.object();
}

void ProcessLogic::setData(const QList<TBBM>& tbbmData, const QList<SPBU>& spbuData, const QJsonObject& roadData)
{
    m_tbbmData = tbbmData;
    m_spbuData = spbuData;
    m_roadDataJson = roadData;
}

void ProcessLogic::runMatrixCalculation()
{
    qDebug() << "Proses perhitungan matriks dimulai di thread:" << QThread::currentThreadId();
    emit progressTextChanged("Memulai perhitungan...");
    emit progressUpdated(0);

    if (m_spbuData.isEmpty()) {
        emit processingFinished(false, "Tidak ada data SPBU untuk diproses.");
        return;
    }

    // Langkah 1: Bangun graf jika ada data jalan
    if (!m_roadDataJson.isEmpty()) {
        buildRoadGraph();
    }

    // Langkah 2: Hitung matriks jarak
    emit progressTextChanged("Menghitung matriks jarak SPBU...");
    int numSpbu = m_spbuData.size();
    QList<QList<double>> distanceMatrix(numSpbu, QList<double>(numSpbu, 0.0));

    int totalSteps = numSpbu * (numSpbu - 1) / 2;
    int currentStep = 0;

    for (int i = 0; i < numSpbu; ++i) {
        for (int j = i + 1; j < numSpbu; ++j) {
            const SPBU& spbu1 = m_spbuData[i];
            const SPBU& spbu2 = m_spbuData[j];

            // TODO: Nanti kita akan tambahkan logika untuk menghitung jarak via graf
            // Untuk sekarang, kita selalu gunakan Haversine
            double dist = haversineDistance(spbu1.koordinat.latitude, spbu1.koordinat.longitude,
                                            spbu2.koordinat.latitude, spbu2.koordinat.longitude);
            distanceMatrix[i][j] = dist;
            distanceMatrix[j][i] = dist; // Matriks simetris

            currentStep++;
            if (totalSteps > 0) {
                emit progressUpdated(static_cast<int>((double)currentStep / totalSteps * 100));
            }
        }
    }

    qDebug() << "Matriks Jarak (Haversine) Selesai Dihitung.";
    emit progressTextChanged("Perhitungan selesai.");
    emit processingFinished(true, "Matriks jarak berhasil dihitung.");
}

void ProcessLogic::buildRoadGraph()
{
    m_roadNodes.clear();
    m_roadEdges.clear();
    QSet<RoadNode> uniqueNodes;

    emit progressTextChanged("Membangun graf jalan...");
    QJsonArray features = m_roadDataJson["features"].toArray();
    int totalFeatures = features.size();
    if (totalFeatures == 0) return;

    for (int i = 0; i < totalFeatures; ++i) {
        QJsonObject feature = features[i].toObject();
        QJsonObject geometry = feature["geometry"].toObject();
        QString type = geometry["type"].toString();
        QJsonArray coordinates = geometry["coordinates"].toArray();
        if (type == "LineString") coordinates = QJsonArray({coordinates});
        if (type == "LineString" || type == "MultiLineString") {
            for (const QJsonValue& lineVal : coordinates) {
                QJsonArray line = lineVal.toArray();
                for (int j = 0; j < line.size() - 1; ++j) {
                    QJsonArray startCoord = line[j].toArray();
                    QJsonArray endCoord = line[j+1].toArray();
                    RoadNode node1(startCoord[0].toDouble(), startCoord[1].toDouble());
                    RoadNode node2(endCoord[0].toDouble(), endCoord[1].toDouble());
                    uniqueNodes.insert(node1);
                    uniqueNodes.insert(node2);
                    RoadEdge edge = {node1, node2, haversineDistance(node1.y(), node1.x(), node2.y(), node2.x())};
                    m_roadEdges.append(edge);
                }
            }
        }
    }
    m_roadNodes = uniqueNodes.values();
    qDebug() << "Pembangunan graf selesai. Total Node:" << m_roadNodes.size() << ", Total Edge:" << m_roadEdges.size();
}

double ProcessLogic::haversineDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double r = 6371.0;
    double lat1_rad = qDegreesToRadians(lat1);
    double lon1_rad = qDegreesToRadians(lon1);
    double lat2_rad = qDegreesToRadians(lat2);
    double lon2_rad = qDegreesToRadians(lon2);
    double dlon = lon2_rad - lon1_rad;
    double dlat = lat2_rad - lat1_rad;
    double a = qPow(qSin(dlat / 2.0), 2) + qCos(lat1_rad) * qCos(lat2_rad) * qPow(qSin(dlon / 2.0), 2);
    double c = 2.0 * qAtan2(qSqrt(a), qSqrt(1.0 - a));
    return r * c;
}
