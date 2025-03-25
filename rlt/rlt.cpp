// rlt.cpp

#include "rlt.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <cmath>

rlt::rlt(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}
rlt::~rlt()
{
    delete yearMonthLineEdit;
    delete stockListWidget;
    delete calculateButton;
    delete correlationTableWidget;
}
void rlt::setupUi()
{
    QWidget *centralWidget=new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout=new QVBoxLayout(centralWidget);
    QHBoxLayout *inputLayout=new QHBoxLayout;
    QLabel *yearMonthLabel=new QLabel("Enter year and month (YYYYMM):");
    yearMonthLineEdit=new QLineEdit;
    inputLayout->addWidget(yearMonthLabel);
    inputLayout->addWidget(yearMonthLineEdit);
    layout->addLayout(inputLayout);
    stockListWidget=new QListWidget(this);
    stockListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    layout->addWidget(stockListWidget);
    calculateButton=new QPushButton("Calculate Correlation", this);
    connect(calculateButton, &QPushButton::clicked, this, &rlt::loadDataAndCalculateCorrelation);
    layout->addWidget(calculateButton);
    correlationTableWidget=new QTableWidget(this);
    layout->addWidget(correlationTableWidget);

    setWindowTitle("Stock Correlation Analysis");
}

void rlt::loadDataAndCalculateCorrelation()//选十支股票
{
    stockListWidget->clear();
    correlationTableWidget->clear();
    QString yearMonth=yearMonthLineEdit->text();
    QString fileName="output.txt";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open file: " + fileName);
        return;
    }
    QTextStream in(&file);
    QVector<QVector<double>> returns;
    QMap<QString, int> stockDataCounts;
    while (!in.atEnd()) {
        QString line=in.readLine();
        QStringList parts=line.split(",");
        if (parts.size()==11 && parts[1].startsWith(yearMonth)) {
            QString stockCode=parts[0];
            if (!stockDataCounts.contains(stockCode))
                stockDataCounts[stockCode]=1;
            else
                stockDataCounts[stockCode]++;
        }
    }
    file.close();
    QMessageBox::information(this, "Info", "Please select 10 stocks from the list.");
    for (auto it=stockDataCounts.constBegin(); it!=stockDataCounts.constEnd(); ++it) {
        QString stockCode=it.key();
        int dataCount=it.value();
        stockListWidget->addItem(QString("%1 (%2)").arg(stockCode).arg(dataCount));//股票数量也要显示，选择数量不相等的股票数据后面皮尔森系数不能算
    }
    while (stockListWidget->selectedItems().size()!=10) {
        qApp->processEvents();
    }
    returns.clear();
    for (QListWidgetItem *item:stockListWidget->selectedItems()) {
        QString stockCode=item->text().split(" ").first();
        QVector<double> prices;
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Failed to open file: " + fileName);
            return;
        }
        in.setDevice(&file);
        while (!in.atEnd()) {
            QString line=in.readLine();
            QStringList parts=line.split(",");
            if (parts.size()==11 && parts[0]==stockCode && parts[1].startsWith(yearMonth))
                prices.append(parts[5].toDouble());
        }
        file.close();
        QVector<double> stockReturns = calculateReturns(prices);
        returns.append(stockReturns);
    }
    calculateAndDisplayCorrelationMatrix(returns);
}

QVector<double> rlt::calculateReturns(const QVector<double> &prices)
{
    QVector<double> returns;
    for (int i=1; i<prices.size(); ++i){
        double dailyReturn=(prices[i]-prices[i-1])/prices[i-1];
        returns.append(dailyReturn);
    }
    return returns;
}

double rlt::calculatePearsonCorrelation(const QVector<double> &returns1, const QVector<double> &returns2)//计算皮尔森
{
    double sumXY=0;
    double sumX=0;
    double sumY=0;
    double sumX2=0;
    double sumY2=0;
    int n=returns1.size();
    for (int i=0; i<n; ++i){
        sumXY+=returns1[i]*returns2[i];
        sumX+=returns1[i];
        sumY+=returns2[i];
        sumX2+=returns1[i]*returns1[i];
        sumY2+=returns2[i]*returns2[i];
    }
    double correlation=(n*sumXY-sumX*sumY)/(sqrt((n*sumX2-sumX*sumX)*(n*sumY2-sumY*sumY)));
    return correlation;
}

void rlt::calculateAndDisplayCorrelationMatrix(const QVector<QVector<double>> &returns)
{
    int numStocks=returns.size();
    correlationTableWidget->setRowCount(numStocks);
    correlationTableWidget->setColumnCount(numStocks);
    QStringList headers;

    for (int i=0; i<numStocks; ++i){
        if (i<stockListWidget->count()) {
            headers << stockListWidget->item(i)->text();
        } else {
            qDebug() << "Error: Not enough items in stockListWidget.";
            return;
        }
    }
    correlationTableWidget->setHorizontalHeaderLabels(headers);
    correlationTableWidget->setVerticalHeaderLabels(headers);
    int expectedLength=returns[0].size();
    for (const auto& ret:returns){
        if (ret.size()!=expectedLength) {
            qDebug() << "Error: Unequal lengths of return sequences.";
            return;
        }
    }
    for (int i=0; i<numStocks; ++i){
        for (int j=0; j<numStocks; ++j){
            double correlation=calculatePearsonCorrelation(returns[i], returns[j]);
            if (std::isnan(correlation)||std::isinf(correlation)){
                qDebug() << "Error: Invalid correlation value.";
                return;
            }
            QTableWidgetItem *item=new QTableWidgetItem(QString::number(correlation));
            correlationTableWidget->setItem(i, j, item);
        }
    }
    plotHeatmap(returns);
}

void rlt::plotHeatmap(const QVector<QVector<double>> &correlationMatrix)
{
    double minCorrelation=std::numeric_limits<double>::max();
    double maxCorrelation=-std::numeric_limits<double>::max();
    for (const auto &row:correlationMatrix) {
        for (double correlation:row) {
            minCorrelation=std::min(minCorrelation, correlation);
            maxCorrelation=std::max(maxCorrelation, correlation);
        }
    }
    QColor minColor=QColor(Qt::yellow);
    QColor maxColor=QColor(Qt::blue);
    int gradientSteps=100;
    double stepSize=(maxCorrelation - minCorrelation)/gradientSteps;
    for (int i=0; i<correlationMatrix.size(); ++i){
        for (int j=0; j<correlationMatrix[i].size(); ++j){
            double correlation=correlationMatrix[i][j];
            int red=static_cast<int>(minColor.red()+(maxColor.red()-minColor.red())*(correlation-minCorrelation)/(maxCorrelation-minCorrelation));
            int green=static_cast<int>(minColor.green()+(maxColor.green()-minColor.green())*(correlation-minCorrelation)/(maxCorrelation-minCorrelation));
            int blue=static_cast<int>(minColor.blue()+(maxColor.blue()-minColor.blue())*(correlation-minCorrelation)/(maxCorrelation-minCorrelation));
            QColor color(red, green, blue);
            QLabel *label=new QLabel(QString::number(correlation), correlationTableWidget);
            label->setAlignment(Qt::AlignCenter);

            double brightness=(0.299*red+0.587*green+0.114*blue)/255;//提高颜色文本和背景颜色对比度
            QColor textColor=(brightness<0.5)?Qt::white:Qt::black;
            label->setStyleSheet("QLabel { background-color : " + color.name() + "; color : " + textColor.name() + "; }");
            correlationTableWidget->setCellWidget(i, j, label);
        }
    }
}



