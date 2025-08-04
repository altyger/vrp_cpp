#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>

class QTabWidget;
class TBBMTab;
class SPBUTab;
class ProcessLogic;
class QThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void muatDataJalan();
    void hitungMatriksJarak();
    void handleProcessingFinished(bool success, const QString& message);

private:
    void setupUI();

    QTabWidget *mainTabWidget;
    TBBMTab *tbbmTab;
    SPBUTab *spbuTab;

    ProcessLogic *logic; // Untuk utilitas seperti load file
    QJsonObject roadData;
    QThread* workerThread;
};
#endif // MAINWINDOW_H
