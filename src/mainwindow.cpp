
#include "../include/mainwindow.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent, std::vector<std::vector<std::string>> data,
                       std::vector<std::vector<std::string>> averages) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    demoName = "Data From Simulation";
    ui->setupUi(this);
    setGeometry(400, 250, 542, 390);


    int numNodes = data.size();
    for (int i = 0; i < numNodes; i++) {
        this->node_nums.push_back(i);
    }

    int rowSize = data[0].size();
    for (int i = 0; i < rowSize; i++) {
        this->times.push_back(i);
    }

    //Convert data for graphs
    for (int i = 0; i < (int)data.size(); i++) {
        std::vector<std::string> dataRow = data[i];
        QVector<double> QDataRow;
        for (int k = 0; k < (int)dataRow.size(); k++) {
            QDataRow.push_back(atof(dataRow[k].c_str()));   //convert the std::string to double
        }
        this->data.push_back(QDataRow);
    }

    for (int i = 0; i < (int)averages.size(); i++) {
        std::vector<std::string> averageRow = averages[i];
        for (int k = 0; k < (int)averageRow.size(); k++) {
            this->averages.push_back(atof(averageRow[k].c_str()));   //convert the std::string to double
        }
    }

    setupTimeSelectionComboBox();
    setupGraphs();
}


void MainWindow::setupGraphs() {
    setupAverageScatterGraph(ui->customPlotScatterAverage);
    setupColorMap(ui->customPlotColorMap);
    setupTimeBarGraph(ui->customPlotTimeBarGraph, 0);
}

void MainWindow::setupAverageScatterGraph(QCustomPlot *customPlot) {
    std::cout << "setting up avg scatter\n";
    // configure axis rect:
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    customPlot->axisRect()->setupFullAxesBox(true);
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Helvetica",9));
    // set locale to english, so we get english decimal separator:
    customPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));
    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    pen.setColor(QColor(180,180,180));

    // add theory curve graph:
    customPlot->addGraph();
    //area under the curve brush
    customPlot->graph(0)->setPen(pen);
    customPlot->graph(0)->setBrush(QBrush(QColor(255,50,30,20)));
    //set style of data line
    pen.setStyle(Qt::DotLine);
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    customPlot->graph(0)->setPen(pen);
    customPlot->graph(0)->setName("Average Curve");



    customPlot->graph(0)->setData(times, averages);

    //final adjustments to graph
    customPlot->graph(0)->rescaleAxes(true);
    // setup look of bottom tick labels:
    customPlot->xAxis->setTickLabelRotation(30);
    customPlot->xAxis->setLabel("Time");
    customPlot->yAxis->setLabel("Avg Load");
    // make top right axes clones of bottom left axes. Looks prettier:
    customPlot->axisRect()->setupFullAxesBox();

}

void MainWindow::setupColorMap(QCustomPlot *customPlot) {
    std::cout << "setting up Color Map\n";
    customPlot->setWindowTitle("3D");

     // configure axis rect:
     // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
     customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
     customPlot->axisRect()->setupFullAxesBox(true);
     customPlot->xAxis->setLabel("Node Numbers");
     customPlot->yAxis->setLabel("Time Value");

     // set up the QCPColorMap:
     QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
     colorMap->data()->setSize(data.size(), times.size());  //we want the map to have nodes * times data points
     colorMap->data()->setRange(QCPRange(0, data.size()), QCPRange(0, times.size())); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions

     //Get the data
     double x, y, z;
     for (int xIndex = 0; xIndex < (int)data.size(); ++xIndex)
     {
           for (int yIndex = 0; yIndex < (int)times.size(); ++yIndex)
           {
                colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);

                z = data[xIndex][yIndex]; // the B field strength of dipole radiation (modulo physical constants)

                colorMap->data()->setCell(xIndex, yIndex, z);
           }
     }

     // add a color scale:
     QCPColorScale *colorScale = new QCPColorScale(customPlot);
     customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
     colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
     colorMap->setColorScale(colorScale); // associate the color map with the color scale
     colorScale->axis()->setLabel("Node Load");

     // set the color gradient of the color map to one of the presets:
     colorMap->setGradient(QCPColorGradient::gpPolar);

     // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
     colorMap->rescaleDataRange();

     // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
     QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
     customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
     colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

     // rescale the key (x) and value (y) axes so the whole color map is visible:
     customPlot->rescaleAxes();
}

void MainWindow::setupTimeBarGraph(QCustomPlot *customPlot, int time) {
     customPlot->clearPlottables();
     customPlot->replot();

     std::cout << "setting up Time bar graph\n";
     customPlot->setWindowTitle("Load on Node n at Time t");
     customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

     QVector<double> loadsAtTimeZero;
     for (int i = 0; i < data.size(); i++) {
        loadsAtTimeZero.push_back(data.at(i).at(time));
     }

     QVector<double> avgAtTimeZero(data.size());
     avgAtTimeZero.fill(averages.at(time));

     // create and configure plottables:
     QCPGraph *graph1 = customPlot->addGraph();
     graph1->setData(node_nums, avgAtTimeZero);
     graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(Qt::white), 9));
     graph1->setPen(QPen(QColor(120, 120, 120), 2));

     QCPBars *bars1 = new QCPBars(customPlot->xAxis, customPlot->yAxis);
     bars1->setData(node_nums, loadsAtTimeZero);
     bars1->setPen(QPen(QColor(10, 140, 70, 160).lighter(170)));
     bars1->setBrush(QColor(10, 140, 70, 160));

     // move bars above graphs and grid below bars:
     customPlot->addLayer("abovemain", customPlot->layer("main"), QCustomPlot::limAbove);
     customPlot->addLayer("belowmain", customPlot->layer("main"), QCustomPlot::limBelow);
     graph1->setLayer("abovemain");
     customPlot->xAxis->grid()->setLayer("belowmain");
     customPlot->yAxis->grid()->setLayer("belowmain");

     // set some pens, brushes and backgrounds:
     customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
     customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
     customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
     customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
     customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
     customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));
     customPlot->xAxis->setTickLabelColor(Qt::white);
     customPlot->yAxis->setTickLabelColor(Qt::white);
     customPlot->xAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
     customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
     customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
     customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
     customPlot->xAxis->grid()->setSubGridVisible(true);
     customPlot->yAxis->grid()->setSubGridVisible(true);
     customPlot->xAxis->grid()->setZeroLinePen(Qt::NoPen);
     customPlot->yAxis->grid()->setZeroLinePen(Qt::NoPen);
     customPlot->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
     customPlot->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
     QLinearGradient plotGradient;
     plotGradient.setStart(0, 0);
     plotGradient.setFinalStop(0, 350);
     plotGradient.setColorAt(0, QColor(80, 80, 80));
     plotGradient.setColorAt(1, QColor(50, 50, 50));
     customPlot->setBackground(plotGradient);
     QLinearGradient axisRectGradient;
     axisRectGradient.setStart(0, 0);
     axisRectGradient.setFinalStop(0, 350);
     axisRectGradient.setColorAt(0, QColor(80, 80, 80));
     axisRectGradient.setColorAt(1, QColor(30, 30, 30));
     customPlot->axisRect()->setBackground(axisRectGradient);

     customPlot->rescaleAxes();
     customPlot->xAxis->setRange(0, data.size());
}

void MainWindow::setupTimeSelectionComboBox() {
    for (int i = 0; i < (int)data[0].size(); i++) {
        ui->comboBoxTime->addItem(QString::fromStdString(std::to_string(i)));
    }
    ui->comboBoxTime->setCurrentIndex(0);

    QObject::connect(ui->comboBoxTime, SIGNAL(currentIndexChanged(int)), this,
                     SLOT(changeRange(int)));
}

void MainWindow::changeRange(int range)
{
    if (range <= (int)times.count() && range >= 0) {
        setupTimeBarGraph(ui->customPlotTimeBarGraph, range);
    }
}


MainWindow::~MainWindow()
{
  delete ui;
}







































