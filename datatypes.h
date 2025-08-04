#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QList>
#include <QPointF>

// Struktur untuk menyimpan data koordinat
struct Koordinat {
    double latitude = 0.0;
    double longitude = 0.0;
};

// Struktur untuk menyimpan data satu mobil tangki
struct ArmadaMobilTangki {
    QString id_mt;
    QString nopol_mt;
    double kapasitas_kl = 0.0;
};

// Struktur utama untuk menyimpan semua informasi tentang satu TBBM
struct TBBM {
    QString id;
    QString nama;
    Koordinat koordinat;
    QList<ArmadaMobilTangki> armada;
};

// Struktur untuk menyimpan semua informasi tentang satu SPBU
struct SPBU {
    QString id;
    QString nama;
    Koordinat koordinat;
    double demand_per_hari = 0.0;
};

// Tipe data untuk Graf Jalan
// QPointF akan menyimpan longitude (x) dan latitude (y)
using RoadNode = QPointF;

struct RoadEdge {
    RoadNode source;
    RoadNode target;
    double weight; // Jarak/bobot edge
};

#endif // DATATYPES_H
