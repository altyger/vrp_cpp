#include "tbbmtab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

TBBMTab::TBBMTab(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

TBBMTab::~TBBMTab()
{
}

QList<TBBM> TBBMTab::getData() const
{
    return tbbmData;
}

void TBBMTab::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(5);
    tableWidget->setHorizontalHeaderLabels({"ID TBBM", "Nama TBBM", "Koordinat TBBM (lat,long)", "Armada", "Total Kapasitas (kL)"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    mainLayout->addWidget(tableWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addButton = new QPushButton("Tambah TBBM");
    QPushButton *deleteButton = new QPushButton("Hapus TBBM");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &TBBMTab::tambahBaris);
    connect(deleteButton, &QPushButton::clicked, this, &TBBMTab::hapusBaris);
    connect(tableWidget, &QTableWidget::itemChanged, this, &TBBMTab::onItemChanged);
}

void TBBMTab::onItemChanged(QTableWidgetItem *item)
{
    if (isInternalChange) return;

    int row = item->row();
    int col = item->column();

    QTableWidgetItem* idItem = tableWidget->item(row, 0);
    if (!idItem) return;
    QString id = idItem->text();

    for (int i = 0; i < tbbmData.size(); ++i) {
        if (tbbmData[i].id == id) {
            switch (col) {
            case 1: // Nama TBBM
                tbbmData[i].nama = item->text();
                qDebug() << "Data TBBM ID" << id << "nama diubah menjadi:" << tbbmData[i].nama;
                break;
            case 2: // Koordinat
            {
                QStringList parts = item->text().split(',');
                if (parts.size() == 2) {
                    bool latOk, lonOk;
                    double lat = parts[0].toDouble(&latOk);
                    double lon = parts[1].toDouble(&lonOk);
                    if (latOk && lonOk) {
                        tbbmData[i].koordinat = {lat, lon};
                        qDebug() << "Data TBBM ID" << id << "koordinat diubah menjadi:" << lat << "," << lon;
                    }
                }
                break;
            }
            }
            return;
        }
    }
}

void TBBMTab::tambahBaris()
{
    isInternalChange = true; // Set flag
    tbbmIdCounter++;
    TBBM tbbmBaru;
    tbbmBaru.id = QString("T%1").arg(tbbmIdCounter);
    tbbmBaru.nama = QString("TBBM Baru %1").arg(tbbmIdCounter);
    tbbmBaru.koordinat = {0.0, 0.0};
    tbbmData.append(tbbmBaru);

    int barisSaatIni = tableWidget->rowCount();
    tableWidget->insertRow(barisSaatIni);

    QTableWidgetItem *idItem = new QTableWidgetItem(tbbmBaru.id);
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

    tableWidget->setItem(barisSaatIni, 0, idItem);
    tableWidget->setItem(barisSaatIni, 1, new QTableWidgetItem(tbbmBaru.nama));
    tableWidget->setItem(barisSaatIni, 2, new QTableWidgetItem(QString("%1,%2").arg(tbbmBaru.koordinat.latitude).arg(tbbmBaru.koordinat.longitude)));
    tableWidget->setCellWidget(barisSaatIni, 3, new QPushButton("Details"));

    QTableWidgetItem *kapasitasItem = new QTableWidgetItem("0.0");
    kapasitasItem->setFlags(kapasitasItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(barisSaatIni, 4, kapasitasItem);

    qDebug() << "TBBM ditambahkan. Total data TBBM:" << tbbmData.count();
    isInternalChange = false; // Reset flag
}

void TBBMTab::hapusBaris()
{
    QModelIndexList barisTerpilih = tableWidget->selectionModel()->selectedRows();

    if (barisTerpilih.isEmpty()) {
        QMessageBox::warning(this, "Peringatan", "Pilih baris yang ingin dihapus terlebih dahulu.");
        return;
    }

    QMessageBox::StandardButton balasan;
    balasan = QMessageBox::question(this, "Konfirmasi Hapus", "Anda yakin ingin menghapus baris yang dipilih?",
                                    QMessageBox::Yes | QMessageBox::No);

    if (balasan == QMessageBox::Yes) {
        for (int i = barisTerpilih.count() - 1; i >= 0; --i) {
            int barisUntukDihapus = barisTerpilih.at(i).row();

            QTableWidgetItem *item = tableWidget->item(barisUntukDihapus, 0);
            if (item) {
                QString idUntukDihapus = item->text();
                tbbmData.removeIf([&](const TBBM &tbbm) {
                    return tbbm.id == idUntukDihapus;
                });
            }
            tableWidget->removeRow(barisUntukDihapus);
        }
        qDebug() << "TBBM dihapus. Total data TBBM:" << tbbmData.count();
    }
}
