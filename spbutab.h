#ifndef SPBUTAB_H
#define SPBUTAB_H
#include <QWidget>
#include <QList>
#include "datatypes.h"
class QTableWidget;
class QTableWidgetItem;
class SPBUTab : public QWidget
{
    Q_OBJECT
public:
    explicit SPBUTab(QWidget *parent = nullptr);
    ~SPBUTab();
    QList<SPBU> getData() const;

private slots:
    void tambahBaris();
    void hapusBaris();
    void onItemChanged(QTableWidgetItem *item);
    void muatDariFile();
private:
    void setupUI();
    void setDataToTable(int row, const SPBU& spbu);
    QTableWidget *tableWidget;
    QList<SPBU> spbuData;
    int spbuIdCounter = 0;
    bool isInternalChange = false;
};
#endif // SPBUTAB_H
