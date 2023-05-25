#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcoreevent.h"
#include "qcustomplot.h"
#include "qtimer.h"

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QByteArray>
#include <numeric>
//#include <QtMath>
//#include "qcustomplot.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    float calculateVelocity(int distance1, int distance2, double timeInterval);

private slots:
    void readSerial();
    void realtimeDataSlot();

    void on_clear_btn_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendor_id = 10755;
    static const quint16 arduino_uno_product_id = 67;
    QByteArray data;
    QString serialBuffer;
    QString distance_data;
    QString angle_data;

    // PLOTTING
    QTimer *dataTimer;
    double sensorData;
    double anglePos;
    double angleRad;
    double x, y;
    const int threshold = 100;

    int test_data;


    // SPEED
    double previousTime = 0;
    double time_Interval;

    int initial_distance = 0;
    int final_distance   = 0;
    float objectVelocity;

    // the amount of data read depends on how fast we want to read the objects speed
    // the higher the number, the less sensitive it is to change
    int store_distance[10];
    int distance_change;



};
#endif // MAINWINDOW_H
