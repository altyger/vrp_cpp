#include "mainwindow.h"
#include "tbbmtab.h"
#include "spbutab.h"
#include "processlogic.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), workerThread(nullptr)
{
    logic = new ProcessLogic(this); // Instance utama untuk utilitas
    setupUI();
    setWindowTitle("Cluster Sim C++");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    if (workerThread && workerThread->isRunning()) {
        workerThread->quit();
        workerThread->wait();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *ribbonLayout = new QHBoxLayout();
    QPushButton *loadRoadButton = new QPushButton("Muat Data Jalan");
    QPushButton* calcMatrixButton = new QPushButton("1. Hitung Matriks Jarak");
    ribbonLayout->addWidget(new QPushButton("Buka Proyek"));
    ribbonLayout->addWidget(new QPushButton("Simpan Proyek"));
    ribbonLayout->addWidget(loadRoadButton);
    ribbonLayout->addStretch();
    ribbonLayout->addWidget(calcMatrixButton);
    ribbonLayout->addWidget(new QPushButton("2. Jalankan Analisis"));
    mainLayout->addLayout(ribbonLayout);

    connect(loadRoadButton, &QPushButton::clicked, this, &MainWindow::muatDataJalan);
    connect(calcMatrixButton, &QPushButton::clicked, this, &MainWindow::hitungMatriksJarak);

    mainTabWidget = new QTabWidget();
    tbbmTab = new TBBMTab();
    spbuTab = new SPBUTab();
    mainTabWidget->addTab(tbbmTab, "Data TBBM");
    mainTabWidget->addTab(spbuTab, "Data SPBU");
    mainTabWidget->addTab(new QWidget(), "Output & Hasil");
    mainTabWidget->addTab(new QWidget(), "Peta");
    mainLayout->addWidget(mainTabWidget);
}

void MainWindow::hitungMatriksJarak()
{
    if (workerThread && workerThread->isRunning()) {
        QMessageBox::warning(this, "Proses Berjalan", "Proses lain sedang berjalan. Harap tunggu.");
        return;
    }
    if (spbuTab->getData().isEmpty()) {
        QMessageBox::warning(this, "Data Tidak Lengkap", "Harap masukkan data SPBU terlebih dahulu.");
        return;
    }

    workerThread = new QThread();
    ProcessLogic* workerLogic = new ProcessLogic(); // Instance baru khusus untuk thread
    workerLogic->setData(tbbmTab->getData(), spbuTab->getData(), roadData);
    workerLogic->moveToThread(workerThread);

    QProgressDialog progressDialog("Menghitung matriks jarak...", "Batal", 0, 100, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    connect(workerThread, &QThread::started, workerLogic, &ProcessLogic::runMatrixCalculation);
    connect(workerLogic, &ProcessLogic::processingFinished, this, &MainWindow::handleProcessingFinished);
    connect(workerLogic, &ProcessLogic::processingFinished, workerThread, &QThread::quit);
    connect(workerLogic, &ProcessLogic::processingFinished, workerLogic, &ProcessLogic::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    connect(workerLogic, &ProcessLogic::progressUpdated, &progressDialog, &QProgressDialog::setValue);
    connect(workerLogic, &ProcessLogic::progressTextChanged, &progressDialog, &QProgressDialog::setLabelText);
    connect(&progressDialog, &QProgressDialog::canceled, workerThread, &QThread::quit);

    workerThread->start();
    progressDialog.exec();
}

void MainWindow::handleProcessingFinished(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "Selesai", message);
    } else {
        QMessageBox::warning(this, "Proses Gagal/Dibatalkan", message);
    }
    workerThread = nullptr; // Set pointer ke null setelah selesai
}

void MainWindow::muatDataJalan()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Buka File Data Jalan", "", "GeoJSON Files (*.geojson *.json)");
    if (filePath.isEmpty()) return;
    bool ok;
    QJsonObject data = logic->loadRoadData(filePath, ok);
    if(ok) {
        roadData = data;
        QMessageBox::information(this, "Berhasil", "Data jalan GeoJSON berhasil dimuat.");
    } else {
        QMessageBox::critical(this, "Error", "Gagal memuat file data jalan.");
    }
}
