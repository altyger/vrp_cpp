#ifndef TBBMTAB_H
#define TBBMTAB_H
#include <QWidget>
#include <QList>
#include "datatypes.h"
class QTableWidget;
class QTableWidgetItem;
class TBBMTab : public QWidget
{
    Q_OBJECT
public:
    explicit TBBMTab(QWidget *parent = nullptr);
    ~TBBMTab();
    QList<TBBM> getData() const;

private slots:
    void tambahBaris();
    void hapusBaris();
    void onItemChanged(QTableWidgetItem *item);
private:
    void setupUI();
    QTableWidget *tableWidget;
    QList<TBBM> tbbmData;
    int tbbmIdCounter = 0;
    bool isInternalChange = false;
};
#endif // TBBMTAB_H
