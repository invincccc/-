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

QVector<double> rlt::calculateReturns(const QVector<double> &prices) {
    QVector<double> returns;
    for (int i = 1; i < prices.size(); ++i) {
        double dailyReturn = (prices[i] - prices[i - 1]) / prices[i - 1];
        returns.append(dailyReturn);
    }
    return returns;
}

double rlt::calculatePearsonCorrelation(const QVector<double> &returns1, const QVector<double> &returns2) {
    int n = std::min(returns1.size(), returns2.size());
    double meanX = 0;
    double meanY = 0;

    for (int i = 0; i < n; ++i) {
        meanX += returns1[i];
        meanY += returns2[i];
    }
    meanX /= n;
    meanY /= n;

    double sumXY = 0;
    double sumX2 = 0;
    double sumY2 = 0;

    for (int i = 0; i < n; ++i) {
        double dx = returns1[i] - meanX;
        double dy = returns2[i] - meanY;
        sumXY += dx * dy;
        sumX2 += dx * dx;
        sumY2 += dy * dy;
    }

    double denominator = std::sqrt(sumX2) * std::sqrt(sumY2);
    if (denominator == 0) {
        qDebug() << "Error: Division by zero in correlation calculation.";
        return 0;
    }
    double correlation = sumXY / denominator;
    return correlation;
}

void rlt::calculateAndDisplayCorrelationMatrix(const QVector<QVector<double>> &returns) {
    int numStocks = returns.size();
    correlationTableWidget->setRowCount(numStocks);
    correlationTableWidget->setColumnCount(numStocks);
    QStringList headers;

    for (int i = 0; i < numStocks; ++i) {
        if (i < stockListWidget->count()) {
            headers << stockListWidget->item(i)->text();
        } else {
            qDebug() << "Error: Not enough items in stockListWidget.";
            return;
        }
    }
    correlationTableWidget->setHorizontalHeaderLabels(headers);
    correlationTableWidget->setVerticalHeaderLabels(headers);

    int minLength = std::numeric_limits<int>::max();
    for (const auto &ret : returns) {
        minLength = std::min(minLength, ret.size());
    }

    if (minLength < 2) {
        qDebug() << "Error: Not enough data points to calculate correlation.";
        return;
    }

    QVector<QVector<double>> correlationMatrix(numStocks, QVector<double>(numStocks, 0));
    for (int i = 0; i < numStocks; ++i) {
        for (int j = 0; j < numStocks; ++j) {
            double correlation = calculatePearsonCorrelation(returns[i].mid(0, minLength), returns[j].mid(0, minLength));
            if (std::isnan(correlation) || std::isinf(correlation)) {
                qDebug() << "Error: Invalid correlation value.";
                return;
            }
            correlationMatrix[i][j] = correlation;
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(correlation));
            correlationTableWidget->setItem(i, j, item);
        }
    }
    plotHeatmap(correlationMatrix);
}

void rlt::plotHeatmap(const QVector<QVector<double>> &correlationMatrix) {
    double minCorrelation = 0.0;
    double maxCorrelation = 1.0;

    QColor minColor = QColor(Qt::cyan); // 浅蓝色
    QColor maxColor = QColor(Qt::blue); // 深蓝色
    int gradientSteps = 255;

    for (int i = 0; i < correlationMatrix.size(); ++i) {
        for (int j = 0; j < correlationMatrix[i].size(); ++j) {
            double correlation = std::abs(correlationMatrix[i][j]);
            double ratio = (correlation - minCorrelation) / (maxCorrelation - minCorrelation);
            int colorIndex = static_cast<int>(ratio * (gradientSteps - 1));

            // 计算颜色插值
            int red = static_cast<int>(minColor.red() + ratio * (maxColor.red() - minColor.red()));
            int green = static_cast<int>(minColor.green() + ratio * (maxColor.green() - minColor.green()));
            int blue = static_cast<int>(minColor.blue() + ratio * (maxColor.blue() - minColor.blue()));
            QColor color = QColor(red, green, blue);

            QLabel *label = new QLabel(QString::number(correlationMatrix[i][j]), correlationTableWidget);
            label->setAlignment(Qt::AlignCenter);

            double brightness = (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
            QColor textColor = (brightness < 0.5) ? Qt::white : Qt::black;
            label->setStyleSheet("QLabel { background-color : " + color.name() + "; color : " + textColor.name() + "; }");
            correlationTableWidget->setCellWidget(i, j, label);
        }
    }

    // 增加一行来放置颜色条
    correlationTableWidget->insertRow(correlationTableWidget->rowCount());

    // 创建颜色条
    int colorBarHeight = 20;
    int colorBarWidth = correlationTableWidget->columnCount() * correlationTableWidget->columnWidth(0);
    QPixmap colorBarPixmap(colorBarWidth, colorBarHeight);
    QPainter painter(&colorBarPixmap);

    for (int i = 0; i < colorBarWidth; ++i) {
        double value = static_cast<double>(i) / colorBarWidth;
        double ratio = value; // 直接用比例
        int red = static_cast<int>(minColor.red() + ratio * (maxColor.red() - minColor.red()));
        int green = static_cast<int>(minColor.green() + ratio * (maxColor.green() - minColor.green()));
        int blue = static_cast<int>(minColor.blue() + ratio * (maxColor.blue() - minColor.blue()));
        QColor color = QColor(red, green, blue);

        painter.setPen(color);
        painter.drawLine(i, 0, i, colorBarHeight);
    }

    QLabel *colorBarLabel = new QLabel();
    colorBarLabel->setPixmap(colorBarPixmap);
    correlationTableWidget->setCellWidget(correlationTableWidget->rowCount() - 1, 0, colorBarLabel);
    correlationTableWidget->setSpan(correlationTableWidget->rowCount() - 1, 0, 1, correlationTableWidget->columnCount());

    // 增加一行来放置数值标记
    correlationTableWidget->insertRow(correlationTableWidget->rowCount());
    int numLabels = 10;
    for (int i = 0; i <= numLabels; ++i) {
        double value = minCorrelation + (maxCorrelation - minCorrelation) * (i / static_cast<double>(numLabels));
        QLabel *valueLabel = new QLabel(QString::number(value), correlationTableWidget);
        valueLabel->setAlignment(Qt::AlignCenter);
        int columnPosition = i * correlationTableWidget->columnCount() / numLabels;
        correlationTableWidget->setCellWidget(correlationTableWidget->rowCount() - 1, columnPosition, valueLabel);
        if (i < numLabels) {
            correlationTableWidget->setSpan(correlationTableWidget->rowCount() - 1, columnPosition, 1, correlationTableWidget->columnCount() / numLabels);
        }
    }
}

