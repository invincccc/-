// rlt.h
#ifndef RLT_H
#define RLT_H
#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVector>
#include "qcustomplot.h"

class rlt : public QMainWindow
{
    Q_OBJECT

public:
    rlt(QWidget *parent = nullptr);
    ~rlt();

private slots:
    void loadDataAndCalculateCorrelation();

private:
    void setupUi();
    QVector<double> calculateReturns(const QVector<double> &prices);
    double calculatePearsonCorrelation(const QVector<double> &returns1, const QVector<double> &returns2);
    void calculateAndDisplayCorrelationMatrix(const QVector<QVector<double>> &returns);
    void plotHeatmap(const QVector<QVector<double>> &correlationMatrix);

    QLineEdit *yearMonthLineEdit;
    QListWidget *stockListWidget;
    QPushButton *calculateButton;
    QTableWidget *correlationTableWidget;
};
#endif // RLT_H
