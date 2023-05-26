#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qcustomplot.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <string>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    dataTimer   = new QTimer(this);
    arduino     = new QSerialPort(this);


    // ******************* 2D PLOTTING *******************
    ui->staticPlot->addGraph();
    ui->staticPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->staticPlot->addGraph();
    ui->staticPlot->graph(1)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->staticPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->staticPlot->graph(1)->setLineStyle(QCPGraph::lsNone);


    ui->staticPlot->xAxis->setRange(-500,500);
    ui->staticPlot->yAxis->setRange(0,500);
    ui->staticPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);



    // ******************* REAL TIME PLOT ********************
    ui->dataPlot->addGraph(); // blue line
    ui->dataPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->dataPlot->xAxis->setTicker(timeTicker);
    ui->dataPlot->axisRect()->setupFullAxesBox();
    ui->dataPlot->yAxis->setRange(-1.2, 500);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->dataPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->dataPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->dataPlot->yAxis2, SLOT(setRange(QCPRange)));

    ui->dataPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);


    // *************** SETUP ARDUINO CONNECTION **************
    // get serial port info
    /* qDebug() << "Number of ports: " << QSerialPortInfo::availablePorts().length() << "\n";
       foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
           qDebug() << "Port Name       : " << serialPortInfo.portName() << "\n";
           qDebug() << "Description     : " << serialPortInfo.description() << "\n";
           qDebug() << "Has vendor id?  : " << serialPortInfo.hasVendorIdentifier() << "\n";
           qDebug() << "Vendor ID       : " << serialPortInfo.vendorIdentifier() << "\n";
           qDebug() << "Has product id? : " << serialPortInfo.hasProductIdentifier() << "\n";
           qDebug() << "Product ID      : " << serialPortInfo.productIdentifier() << "\n";
       } */

    //identify the port the arduino is on
    bool arduino_is_available = false;
    QString arduino_uno_port_name;

    // for each available serial port
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        //  check if the serialport has both a product identifier and a vendor identifier
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            //  check if the product ID and the vendor ID match those of the arduino uno
            if((serialPortInfo.productIdentifier() == arduino_uno_product_id) && (serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id)){
                arduino_is_available  = true; // arduino uno is available on this port
                arduino_uno_port_name = serialPortInfo.portName();
            }
        }
    }

    // open and configure the arduino port if available
    if(arduino_is_available){
        qDebug() << "Found the arduino port...\n";
        arduino->setPortName(arduino_uno_port_name);    // COM4
        arduino->open(QSerialPort::ReadOnly);
        arduino->setBaudRate(QSerialPort::Baud115200);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);

        // listen to the signal from arduino called readyread to the slot readSerial
        connect(arduino, &QSerialPort::readyRead, this, &MainWindow::readSerial);
        arduino->setDataTerminalReady(true);

    } else {
        qDebug() << "Couldn't find the correct port for the arduino.\n";
        QMessageBox::information(this, "Serial Port Error", "Couldn't open serial port to arduino.");
    }

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer->start(0); // Interval 0 means to refresh as fast as possible
}



MainWindow::~MainWindow()
{
    if(arduino->isOpen()){
        arduino->close(); // Close the serial port if it's open.
    }
    delete ui;
}



void MainWindow::readSerial(){

    /*
     *  To avoid splitting the data received from the readyRead() function,
     *  the message arrived are split into parts to buffer the serial data
     *
    */

    QStringList buffer_split = serialBuffer.split(",");
    if(buffer_split.length() < 5){
        data = arduino->readAll();
        serialBuffer = serialBuffer + QString::fromStdString(data.toStdString());
        data.clear();
    } else {
        serialBuffer = "";

        //distance_data = buffer_split[1];

        /***** TESTING 2D MAP  ****/
        distance_data   = buffer_split[2];  // take the distance value
        angle_data      = buffer_split[1];  // take the angle value
        //qDebug() << "distance data:     " << distance_data << "\n";
        //qDebug() << "angle data:        " << angle_data << "\n";

    }

    static bool conversionOK = true;
    sensorData  = distance_data.toDouble(&conversionOK);
    //qDebug() << "distance data:     " << sensorData << "\n";

    anglePos    = angle_data.toDouble(&conversionOK);
    //qDebug() << "distance:  " << sensorData << "cm" << "\t" << "angle:  " << anglePos << "degrees" << "\n";

    // use the code below for 2D mapping
    // calculate x and y coordinates
    angleRad = anglePos*(M_PI/180.0);
    x = sensorData * cos(angleRad);
    y = sensorData * sin(angleRad);

    //qDebug() << "distance: " << sensorData << "     Angle(rad): " << angleRad << "      Angle(deg): " << anglePos << "      x  : " << x << "     y  :" << y << "\n";

    //blue zone, if the distance is above the threshold
    if(y > threshold){
        ui->staticPlot->graph(0)->addData(x, y);
        ui->staticPlot->graph(0)->setPen(QPen(Qt::blue));
    //red zone, if the distance is below the threshold
    } else if (y <= threshold){
        ui->staticPlot->graph(1)->addData(x, y);
        ui->staticPlot->graph(1)->setPen(QPen(Qt::red));
    }

//     testing for specific angles
//    if (anglePos==135){
//        test_data = sensorData;
//        ui->display_distance->display(sensorData);
//    }

    ui->staticPlot->replot();
    ui->staticPlot->update();

    data.clear();
}


// Real time data
void MainWindow::realtimeDataSlot(){
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of program, in seconds
    static double lastPointKey = 0;     // initialise last recorded time
    static float minTimeInterval = 0.05;
    // static int mins = 60;

    // velocity
//        static int i = 0;   // index value
//        static int maxIndex = 10;
//        previousTime = 0;   // previous time for velocity
//        store_distance[0] = sensorData;  //initial distance


    // time elapsed - last time
    if (key-lastPointKey > minTimeInterval){ // at most add point every 0.05 ms
        // add data to line:
        ui->dataPlot->graph(0)->addData(key, sensorData); // add data from the sensor
        ui->display_distance->display(sensorData);

        lastPointKey = key; // update the new last time

        // velocity
//        i++; // increment index value
//        store_distance[i] = sensorData; // store the curernt distance

//        // reset if array exceeds the limit
//        if(i >= maxIndex){
//            i = 0;
//            store_distance[i] = sensorData;
//        }
    }

    // read data each minute
    //    if(key == mins){
    //        qDebug() << "distance after 1 min:  " << test_data;
    //    } else if(key == mins*2) {
    //        qDebug() << "distance after 2 mins: " << test_data;
    //    } else if(key == mins*3) {
    //        qDebug() << "distance after 3 mins: " << test_data;
    //    } else if(key == mins*4) {
    //        qDebug() << "distance after 4 mins: " << test_data;
    //    } else if(key == mins*5) {
    //        qDebug() << "distance after 5 mins: " << test_data;
    //    }


    //initial_distance    = store_distance[0];
    //final_distance      = store_distance[9];

    // approximate time interval = time interval to read each data (0.05) * total number of data (10)
    //objectVelocity      = calculateVelocity(initial_distance, final_distance, 0.5);
    //objectVelocity      = calculateVelocity(store_distance[0], store_distance[maxIndex-1], 0.5);

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->dataPlot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->dataPlot->replot();


    /* calculate frames per second:
    //    static double lastFpsKey;
    //    static int frameCount;
    //    ++frameCount;
    //    if (key-lastFpsKey > 2) // average fps over 2 seconds
    //    {
    //        ui->statusbar->showMessage(
    //                    QString("%1 FPS, Total Data points: %2")
    //                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
    //                    .arg(ui->dataPlot->graph(0)->data()->size())
    //                    , 0);
    //        lastFpsKey = key;
    //        frameCount = 0;
    //    } */
}

float MainWindow::calculateVelocity(int distance1, int distance2, double timeInterval){
    float velocity = (distance2 - distance1)/timeInterval;
    ui->display_speed->display(velocity);
    // qDebug() << "D1 =" << distance1 << "   D2 =" << distance2 << " T =" << timeInterval << "   velocity:" << velocity << "cm/s";
    return velocity;
}


// clear the data on the plot
void MainWindow::on_clear_btn_clicked()
{
    ui->dataPlot->graph(0)->data()->clear();
    ui->staticPlot->graph(0)->data()->clear();
    ui->staticPlot->graph(1)->data()->clear();
}

