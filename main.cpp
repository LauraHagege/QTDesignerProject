#include "mainwindow.h"
#include <QApplication>
#include "dialog.h"
#include <QMessageBox>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // dialog box to select the DICOMDIR file
    Dialog dialogbox;
    dialogbox.setModal(true);
    dialogbox.exec();

    //Get the selected path
    QString path=dialogbox.getFilepath();



    MainWindow w;

    //create window architecture
    // find images according to the given path
    w.constructWindow(path);


    //
    w.showMaximized();




    return a.exec();
}
