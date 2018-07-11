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

    //if(dialogbox.exec()){
        QString path=dialogbox.getFilepath();
       // QMessageBox::information(a,QObject::tr("Directory path"), path);
        //cout << path.toStdString() << endl;
   // }

    /*const char *file = "/Users/laurah/Desktop/CTAbdomen/DICOMDAT/SDY00000/SRS00000/IMG00000";
    DicomImage *image = new DicomImage(file);*/

    MainWindow w;
    w.setFilepath(path);



    w.showMaximized();




    return a.exec();
}
