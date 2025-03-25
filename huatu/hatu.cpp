#include "hatu.h"
Hatu::Hatu(QWidget *parent)
    : QMainWindow(parent), plot(nullptr)
{
    setupUi();
    connectSignalsSlots();
    buildIndexMap();
}

void Hatu::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    stockCodeLineEdit = new QLineEdit(this);
    stockCodeLineEdit->setPlaceholderText("Enter stock code");
    layout->addWidget(stockCodeLineEdit);

    monthLineEdit = new QLineEdit(this);
    monthLineEdit->setPlaceholderText("Enter month");
    layout->addWidget(monthLineEdit);

    showDataButton = new QPushButton("Show Data", this);
    layout->addWidget(showDataButton);

    plot = new QCustomPlot(this);
    layout->addWidget(plot);

}

void Hatu::connectSignalsSlots()
{
    connect(showDataButton, &QPushButton::clicked, this, &Hatu::showStockData);
}

void Hatu::buildIndexMap()
{
    QFile indexFile("index.txt");
    if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open index.txt";
        return;
    }
    QTextStream in(&indexFile);
    while (!in.atEnd())
    {
        QString line=in.readLine();
        QStringList parts=line.split(",");
        if (parts.size()==3)
        {
            QString stockCode=parts[0];
            QString month=parts[1];
            qint64 offset=parts[2].toLongLong();
            indexMap[stockCode + month] = offset;
        }
    }

    indexFile.close();
}

void Hatu::showStockData()
{
    QString stockCode = stockCodeLineEdit->text();
    QString month = monthLineEdit->text();
    QString key = stockCode + month;
    if (!indexMap.contains(key))
    {
        qDebug() << "Data not found for the specified stock code and month";
        return;
    }

    qint64 fileOffset = indexMap[key];
    qDebug() << "File Offset:" << fileOffset;

    QStringList stockData = readStockData(fileOffset, false);
    QStringList nextMonthData = readStockData(fileOffset, true);

    QVector<double> closePrices;
    for(const QString& dataStr : stockData)
    {
        QStringList parts = dataStr.split(",");
        if (parts.size() == 6)
        {
            double priceClose = parts[4].toDouble();
            qDebug() << "Close Price:" << priceClose;
            closePrices.push_back(priceClose);
        }
        else
        {
            qDebug() << "Unexpected data format:" << dataStr;
        }
    }

    if (closePrices.size() > 0)
    {
        double nextMonthClose = linearRegression(closePrices);
        qDebug() << "Next Month Predicted Close Price: " << nextMonthClose;

        QVector<double> actualPrices;
        for(const QString& dataStr : nextMonthData)
        {
            QStringList parts = dataStr.split(",");
            if (parts.size() == 6)
            {
                double priceClose = parts[4].toDouble();
                actualPrices.push_back(priceClose);
            }
            else
            {
                qDebug() << "Unexpected data format:" << dataStr;
            }
        }

        if (!actualPrices.isEmpty())
        {
            // 创建新的窗口显示 comparisonPlot
            QMainWindow *comparisonWindow = new QMainWindow();
            QCustomPlot *comparisonPlot = new QCustomPlot(comparisonWindow);
            comparisonWindow->setCentralWidget(comparisonPlot);
            comparisonWindow->resize(800, 600);
            comparisonWindow->show();
            comparisonPlot->setWindowTitle("Comparison Graph");
            plotComparisonGraph(actualPrices, nextMonthClose, comparisonPlot);

            double rmse = calculateRMSE(actualPrices, QVector<double>(actualPrices.size(), nextMonthClose));
            qDebug() << "RMSE: " << rmse;
        }
        else
        {
            qDebug() << "No data available for the next month";
        }
    }
    else
    {
        qDebug() << "Insufficient data for prediction";
    }

    plotStockData(stockData);
}

QStringList Hatu::readStockData(qint64 fileOffset, bool getNextMonthData)//提取数据
{
    QStringList stockData;

    QFile dataFile("output.txt");
    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open output.txt";
        return stockData;
    }

    QTextStream in(&dataFile);
    if (!dataFile.seek(fileOffset))
    {
        qDebug() << "Failed to seek to file offset";
        return stockData;
    }

    QString line = in.readLine();
    while (!line.isNull())
    {
        QStringList parts = line.split(",");
        if (parts.size() == 11)
        {
            QString stockNumber = parts[0];
            if (stockNumber == stockCodeLineEdit->text())
            {
                QString date = parts[1];
                QString dataMonth = date.mid(0, 6);

                if ((getNextMonthData && dataMonth == getNextMonth(monthLineEdit->text())) ||
                    (!getNextMonthData && dataMonth == monthLineEdit->text()))
                {
                    double priceOpen = parts[2].toDouble();
                    double priceHigh = parts[3].toDouble();
                    double priceLow = parts[4].toDouble();
                    double priceClose = parts[5].toDouble();
                    double amount = parts[10].toDouble(); // 读取交易金额
                    stockData.append(date + "," +
                                     QString::number(priceOpen) + "," +
                                     QString::number(priceHigh) + "," +
                                     QString::number(priceLow) + "," +
                                     QString::number(priceClose) + "," +
                                     QString::number(amount)); // 将交易金额添加到stockData中
                }
            }
        }
        fileOffset = dataFile.pos();
        line = in.readLine();
    }

    dataFile.close();
    return stockData;
}


QString Hatu::getNextMonth(const QString &currentMonth)
{
    QDate date=QDate::fromString(currentMonth, "yyyyMM");
    date=date.addMonths(1);
    return date.toString("yyyyMM");
}

void Hatu::plotStockData(const QStringList &stockData) // 展示K线图和交易金额柱形图
{
    plot->clearPlottables();
    plot->plotLayout()->clear();

    // 创建两个轴矩形，一个用于K线图，一个用于交易金额柱形图
    QCPAxisRect *mainAxisRect = new QCPAxisRect(plot);
    QCPAxisRect *amountAxisRect = new QCPAxisRect(plot);
    plot->plotLayout()->addElement(0, 0, mainAxisRect);
    plot->plotLayout()->addElement(1, 0, amountAxisRect);

    // 同步时间轴
    mainAxisRect->setupFullAxesBox(true);
    amountAxisRect->setupFullAxesBox(true);
    amountAxisRect->axis(QCPAxis::atBottom)->setTicker(mainAxisRect->axis(QCPAxis::atBottom)->ticker());
    connect(mainAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), amountAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));

    QVector<double> time_list, priceOpen_list, priceHigh_list, priceLow_list, priceClose_list, amount_list;
    for (const QString& dataStr : stockData)
    {
        QStringList parts = dataStr.split(",");
        if (parts.size() == 6)
        {
            QString date = parts[0];
            double priceOpen = parts[1].toDouble();
            double priceHigh = parts[2].toDouble();
            double priceLow = parts[3].toDouble();
            double priceClose = parts[4].toDouble();
            double amount = parts[5].toDouble(); // 获取交易金额

            QDateTime dateTime = QDateTime::fromString(date, "yyyyMMdd");
            double time = dateTime.toSecsSinceEpoch() + 43200; // 修改为toSecsSinceEpoch并加上半天的秒数（43200）以确保时间在中间

            time_list.push_back(time);
            priceOpen_list.push_back(priceOpen);
            priceHigh_list.push_back(priceHigh);
            priceLow_list.push_back(priceLow);
            priceClose_list.push_back(priceClose);
            amount_list.push_back(amount); // 添加到amount列表
        }
    }

    // 创建并配置K线图
    QCPFinancial *candlesticks = new QCPFinancial(mainAxisRect->axis(QCPAxis::atBottom), mainAxisRect->axis(QCPAxis::atLeft));
    candlesticks->setName("Candlestick");
    candlesticks->setChartStyle(QCPFinancial::csCandlestick);

    for (int i = 0; i < time_list.size(); ++i)
    {
        double time = time_list[i];
        double priceOpen = priceOpen_list[i];
        double priceHigh = priceHigh_list[i];
        double priceLow = priceLow_list[i];
        double priceClose = priceClose_list[i];

        candlesticks->data()->add(QCPFinancialData(time, priceOpen, priceHigh, priceLow, priceClose));
    }

    candlesticks->setWidth(3600 * 24); // 调整蜡烛宽度以确保对齐
    candlesticks->setTwoColored(true);
    candlesticks->setBrushPositive(QColor(180, 90, 90));
    candlesticks->setBrushNegative(QColor(90, 180, 90));
    candlesticks->setPenPositive(QPen(QColor(0, 0, 0)));
    candlesticks->setPenNegative(QPen(QColor(0, 0, 0)));

    // 创建交易金额的柱形图
    QCPBars *amountBars = new QCPBars(amountAxisRect->axis(QCPAxis::atBottom), amountAxisRect->axis(QCPAxis::atLeft));
    amountBars->setName("Transaction Amount");
    amountBars->setPen(QPen(QColor(0, 168, 140).lighter(130)));
    amountBars->setBrush(QColor(0, 168, 140));
    amountBars->setWidth(3600 * 24); // 设置柱形图宽度
    amountBars->setData(time_list, amount_list);

    // 设置时间轴格式和密集度
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setDateTimeFormat("yyyy-MM-dd");
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setTickCount(20); // 增加刻度数量以使时间轴标签更密集
    mainAxisRect->axis(QCPAxis::atBottom)->setTicker(dateTimeTicker);
    mainAxisRect->axis(QCPAxis::atBottom)->setTickLabelRotation(15);
    amountAxisRect->axis(QCPAxis::atBottom)->setTicker(dateTimeTicker);
    amountAxisRect->axis(QCPAxis::atBottom)->setTickLabelRotation(15);

    // 设置时间轴和价格轴范围
    mainAxisRect->axis(QCPAxis::atBottom)->setRange(*std::min_element(time_list.begin(), time_list.end()) - 3600 * 12,
                                                    *std::max_element(time_list.begin(), time_list.end()) + 3600 * 12);
    mainAxisRect->axis(QCPAxis::atLeft)->setRange(*std::min_element(priceLow_list.begin(), priceLow_list.end()),
                                                  *std::max_element(priceHigh_list.begin(), priceHigh_list.end()));
    amountAxisRect->axis(QCPAxis::atLeft)->setRange(0, *std::max_element(amount_list.begin(), amount_list.end()));

    plot->replot();
}

double Hatu::linearRegression(const QVector<double> &closePrices)
{
    // 构建自变量（月份）和因变量（收盘价）
    QVector<double> months;
    for (int i=1; i<=closePrices.size(); ++i)
    {
        months.push_back(i);
    }

    // 计算线性回归的截距和斜率
    double sumX=0, sumY=0, sumXY=0, sumX2=0;
    for (int i=0; i<months.size(); ++i)
    {
        sumX+=months[i];
        sumY+=closePrices[i];
        sumXY+=months[i]*closePrices[i];
        sumX2+=months[i]*months[i];
    }

    double n=months.size();
    double b=(n*sumXY-sumX*sumY)/(n*sumX2-sumX*sumX);
    return b*(n+1)+closePrices.last(); // 预测下个月的收盘价
}
void Hatu::plotComparisonGraph(const QVector<double> &actualPrices, double predictedPrice, QCustomPlot *plot)
{
    if (!plot) {
        qDebug() << "Plot pointer is null. Aborting plot.";
        return;
    }
    // 清除现有的图形
    plot->clearGraphs();
    // 创建实际价格曲线
    QVector<double> days(actualPrices.size());
    for (int i=0; i<actualPrices.size(); ++i)
    {
        days[i]=i + 1;
    }
    QCPGraph *actualGraph=plot->addGraph();
    actualGraph->setData(days, actualPrices);
    actualGraph->setName("Actual Prices");
    actualGraph->setPen(QPen(Qt::blue));
    // 创建预测价格曲线
    QVector<double> predictedPrices(actualPrices.size(), predictedPrice);
    QCPGraph *predictedGraph=plot->addGraph();
    predictedGraph->setData(days, predictedPrices);
    predictedGraph->setName("Predicted Prices");
    predictedGraph->setPen(QPen(Qt::red));
    // 创建误差曲线
    QVector<double> errors(actualPrices.size());
    for (int i=0; i<actualPrices.size(); ++i)
    {
        errors[i]=predictedPrice-actualPrices[i];
    }

    QCPGraph *errorGraph=plot->addGraph();
    errorGraph->setData(days, errors);
    errorGraph->setName("Prediction Errors");
    errorGraph->setPen(QPen(Qt::green));

    plot->rescaleAxes();
    plot->replot();
}

double Hatu::calculateRMSE(const QVector<double> &actual, const QVector<double> &predicted)//计算RMSE
{
    double sumSquaredError=0;
    for (int i=0; i<actual.size(); ++i)
    {
        double error=actual[i]-predicted[i];
        sumSquaredError+=error*error;
    }
    double meanSquaredError=sumSquaredError/actual.size();
    return qSqrt(meanSquaredError);
}
