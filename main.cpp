#include "mainwindow.h"
#include <QApplication>
#include "dialog.h"
#include <QMessageBox>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    Dialog dialogbox;
    dialogbox.setModal(true);
    dialogbox.exec();


    QString path=dialogbox.getFilepath();

    //const char *path = "/Users/laurah/Desktop/CTAbdomen/DICOMDAT/DICOMDIR";


    MainWindow w;
    w.setFilepath(path);



    w.showMaximized();




    return a.exec();
}
