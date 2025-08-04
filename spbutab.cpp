#include "spbutab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SPBUTab::SPBUTab(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

SPBUTab::~SPBUTab()
{
}

QList<SPBU> SPBUTab::getData() const
{
    return spbuData;
}

void SPBUTab::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"ID SPBU", "Nama SPBU", "Koordinat SPBU (lat,long)", "Demand/Hari (kL)"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    mainLayout->addWidget(tableWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *loadButton = new QPushButton("Muat dari File...");
    QPushButton *addButton = new QPushButton("Tambah SPBU");
    QPushButton *deleteButton = new QPushButton("Hapus SPBU");

    buttonLayout->addWidget(loadButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);

    mainLayout->addLayout(buttonLayout);

    connect(loadButton, &QPushButton::clicked, this, &SPBUTab::muatDariFile);
    connect(addButton, &QPushButton::clicked, this, &SPBUTab::tambahBaris);
    connect(deleteButton, &QPushButton::clicked, this, &SPBUTab::hapusBaris);
    connect(tableWidget, &QTableWidget::itemChanged, this, &SPBUTab::onItemChanged);
}

void SPBUTab::muatDariFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Buka File Data SPBU", "", "CSV Files (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Gagal membuka file: " + file.errorString());
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Konfirmasi Muat Data",
                                                              "Ini akan menghapus semua data SPBU saat ini dan menggantinya dengan data dari file. Lanjutkan?",
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::No) {
        file.close();
        return;
    }

    isInternalChange = true;
    spbuData.clear();
    tableWidget->setRowCount(0);
    spbuIdCounter = 0;

    QTextStream in(&file);
    if (!in.atEnd()) {
        in.readLine();
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 3) {
            spbuIdCounter++;
            SPBU spbuBaru;
            spbuBaru.id = QString("D%1").arg(spbuIdCounter);
            spbuBaru.nama = fields[0];
            spbuBaru.koordinat = {fields[1].toDouble(), fields[2].toDouble()};
            spbuBaru.demand_per_hari = (fields.size() > 3) ? fields[3].toDouble() : 0.0;
            spbuData.append(spbuBaru);

            int newRow = tableWidget->rowCount();
            tableWidget->insertRow(newRow);
            setDataToTable(newRow, spbuBaru);
        }
    }

    file.close();
    isInternalChange = false;
    QMessageBox::information(this, "Berhasil", QString("Berhasil memuat %1 data SPBU.").arg(spbuData.size()));
    qDebug() << "Data SPBU dimuat dari file. Total:" << spbuData.count();
}


void SPBUTab::onItemChanged(QTableWidgetItem *item)
{
    if (isInternalChange) return;
    int row = item->row();
    int col = item->column();
    QTableWidgetItem* idItem = tableWidget->item(row, 0);
    if (!idItem) return;
    QString id = idItem->text();

    for (int i = 0; i < spbuData.size(); ++i) {
        if (spbuData[i].id == id) {
            switch (col) {
            case 1: spbuData[i].nama = item->text(); break;
            case 2: {
                QStringList parts = item->text().split(',');
                if (parts.size() == 2) {
                    spbuData[i].koordinat = {parts[0].toDouble(), parts[1].toDouble()};
                }
                break;
            }
            case 3: spbuData[i].demand_per_hari = item->text().toDouble(); break;
            }
            qDebug() << "Data SPBU ID" << id << "diperbarui.";
            return;
        }
    }
}

void SPBUTab::setDataToTable(int row, const SPBU& spbu)
{
    QTableWidgetItem *idItem = new QTableWidgetItem(spbu.id);
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

    tableWidget->setItem(row, 0, idItem);
    tableWidget->setItem(row, 1, new QTableWidgetItem(spbu.nama));
    tableWidget->setItem(row, 2, new QTableWidgetItem(QString("%1,%2").arg(spbu.koordinat.latitude).arg(spbu.koordinat.longitude)));
    tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(spbu.demand_per_hari)));
}


void SPBUTab::tambahBaris()
{
    isInternalChange = true;
    spbuIdCounter++;
    SPBU spbuBaru;
    spbuBaru.id = QString("D%1").arg(spbuIdCounter);
    spbuBaru.nama = QString("SPBU Baru %1").arg(spbuIdCounter);
    spbuBaru.koordinat = {0.0, 0.0};
    spbuBaru.demand_per_hari = 0.0;
    spbuData.append(spbuBaru);

    int barisSaatIni = tableWidget->rowCount();
    tableWidget->insertRow(barisSaatIni);
    setDataToTable(barisSaatIni, spbuBaru);

    qDebug() << "SPBU ditambahkan. Total data SPBU:" << spbuData.count();
    isInternalChange = false;
}

void SPBUTab::hapusBaris()
{
    QModelIndexList barisTerpilih = tableWidget->selectionModel()->selectedRows();
    if (barisTerpilih.isEmpty()) {
        QMessageBox::warning(this, "Peringatan", "Pilih baris yang ingin dihapus terlebih dahulu.");
        return;
    }
    QMessageBox::StandardButton balasan = QMessageBox::question(this, "Konfirmasi Hapus", "Anda yakin ingin menghapus baris yang dipilih?", QMessageBox::Yes | QMessageBox::No);
    if (balasan == QMessageBox::Yes) {
        for (int i = barisTerpilih.count() - 1; i >= 0; --i) {
            int barisUntukDihapus = barisTerpilih.at(i).row();
            QTableWidgetItem *item = tableWidget->item(barisUntukDihapus, 0);
            if (item) {
                spbuData.removeIf([&](const SPBU &spbu) { return spbu.id == item->text(); });
            }
            tableWidget->removeRow(barisUntukDihapus);
        }
        qDebug() << "SPBU dihapus. Total data SPBU:" << spbuData.count();
    }
}
