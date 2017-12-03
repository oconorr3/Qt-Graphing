
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QTimer>
#include "qcustomplot.h" // the header file of QCustomPlot. Don't forget to add it to your project, if you use an IDE, so it gets compiled.

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent, std::vector<std::vector<std::string>> data,
                       std::vector<std::vector<std::string>> averages);
  ~MainWindow();
  
  void setupGraphs();
  
public Q_SLOTS:
  void changeRange(int);
  //void realtimeDataSlot();
  //void bracketDataSlot();
  //void screenShot();
  //void allScreenShots();

private:
  Ui::MainWindow *ui;
  QString demoName;
  QVector<QVector<double>> data;
  QVector<double> averages;
  QVector<double> times;
  QVector<double> node_nums;

  void setupTimeSelectionComboBox();
  void setupAverageScatterGraph(QCustomPlot *customPlot);
  void setupColorMap(QCustomPlot *customPlot);
  void setupTimeBarGraph(QCustomPlot *customPlot, int time);
};

#endif // MAINWINDOW_H
