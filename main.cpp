#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;    
    //w.setFixedSize(343, 146);
    //w.setWindowTitle("Distance");
    w.show();
    return a.exec();
}
