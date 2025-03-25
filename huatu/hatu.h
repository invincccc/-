#ifndef HATU_H
#define HATU_H
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "qcustomplot.h"
#include <QListWidget>
class Hatu : public QMainWindow
{
    Q_OBJECT

public:
    Hatu(QWidget *parent = nullptr);

private slots:
    void showStockData();
private:
    void setupUi();
    QLineEdit *stockCodeLineEdit;
    QLineEdit *monthLineEdit;
    QPushButton *showDataButton;
    QPushButton *showHeatmapButton;
    QCustomPlot *plot;
    void buildIndexMap();
    QMap<QString, qint64> indexMap;
    QListWidget *stockListWidget;
    void connectSignalsSlots();
    qint64 retrieveFileOffset(const QString &stockCode, const QString &month);
    QStringList readStockData(qint64 fileOffset, bool getNextMonthData);
    void plotStockData(const QStringList &stockData);
    double linearRegression(const QVector<double> &closePrices);
    void plotComparisonGraph(const QVector<double> &actualPrices, double predictedPrice, QCustomPlot *plot);
    QString getNextMonth(const QString &currentMonth);
    double calculateRMSE(const QVector<double> &actual, const QVector<double> &predicted);
};
#endif // HATU_H

